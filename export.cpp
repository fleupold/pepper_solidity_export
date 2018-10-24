#include <iostream>
#include <iomanip>
#include <fstream>

#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

void replaceAll(std::string& haystack, const std::string needle, const std::string target) {
	std::string::size_type n = 0;
	while ( ( n = haystack.find( needle, n ) ) != std::string::npos ) {
		haystack.replace( n, needle.size(), target);
		n += target.size();
	}
}

std::string HexStringFromLibsnarkBigint(libff::bigint<libsnark::default_r1cs_ppzksnark_pp::Fp_type::num_limbs> _x){
	uint8_t x[32];
	for (unsigned i = 0; i < 4; i++) {
		for (unsigned j = 0; j < 8; j++) {
			x[i * 8 + j] = uint8_t(uint64_t(_x.data[3 - i]) >> (8 * (7 - j)));
		}
	}

	std::stringstream ss;
	ss << std::setfill('0');
	for (unsigned i = 0; i<32; i++) {
		ss << std::hex << std::setw(2) << (int)x[i];
	}

	std::string str = ss.str();
	return str.erase(0, std::min(str.find_first_not_of('0'), str.size()-1));
}

std::string outputPointG1AffineAsHex(libff::G1<libsnark::default_r1cs_ppzksnark_pp> _p, bool escape)
{
	libff::G1<libsnark::default_r1cs_ppzksnark_pp> aff = _p;
	aff.to_affine_coordinates();
	return
		(escape ? "[\"0x" : "0x") +
		HexStringFromLibsnarkBigint(aff.X.as_bigint()) +
		(escape ? "\", \"0x" : ", 0x") +
		HexStringFromLibsnarkBigint(aff.Y.as_bigint()) +
		(escape ? "\"]" : "");
}

std::string outputPointG2AffineAsHex(libff::G2<libsnark::default_r1cs_ppzksnark_pp> _p, bool escape)
{
	libff::G2<libsnark::default_r1cs_ppzksnark_pp> aff = _p;
	aff.to_affine_coordinates();
	return
		(escape ? "[[\"0x" : "[0x") +
		HexStringFromLibsnarkBigint(aff.X.c1.as_bigint()) +
		(escape ? "\", \"0x" : ", 0x") +
		HexStringFromLibsnarkBigint(aff.X.c0.as_bigint()) + 
		(escape ? "\"], [\"0x" : "], [0x") +
		HexStringFromLibsnarkBigint(aff.Y.c1.as_bigint()) +
		(escape ? "\", \"0x" : ", 0x") +
		HexStringFromLibsnarkBigint(aff.Y.c0.as_bigint()) + 
		(escape ? "\"]]" : "]");
}

void export_contract(char *verification_key_fn) {
	std::cout << "loading vk from file: " << verification_key_fn << std::endl;
    	std::ifstream vkey_stream(verification_key_fn);
	if (!vkey_stream.good()) {
                std::cerr << "ERROR: " << verification_key_fn << " not found. " << std::endl;
                exit(1);
        }
	libsnark::r1cs_ppzksnark_verification_key<libsnark::default_r1cs_ppzksnark_pp> vk;
	vkey_stream >> vk;
	vkey_stream.close();

	std::ifstream template_stream("template.sol");
	if (!template_stream.good()) {
		std::cerr << "ERROR: template.sol not found." << std::endl;
	}
	std::string template_string((std::istreambuf_iterator<char>(template_stream)), std::istreambuf_iterator<char>());
	unsigned icLength = vk.encoded_IC_query.rest.indices.size() + 1;

	replaceAll(template_string, "<%vk_a%>", outputPointG2AffineAsHex(vk.alphaA_g2, false));
	replaceAll(template_string, "<%vk_b%>", outputPointG1AffineAsHex(vk.alphaB_g1, false));
	replaceAll(template_string, "<%vk_c%>", outputPointG2AffineAsHex(vk.alphaC_g2, false));
	replaceAll(template_string, "<%vk_g%>", outputPointG2AffineAsHex(vk.gamma_g2, false));
	replaceAll(template_string, "<%vk_gb1%>", outputPointG1AffineAsHex(vk.gamma_beta_g1, false));
	replaceAll(template_string, "<%vk_gb2%>", outputPointG2AffineAsHex(vk.gamma_beta_g2, false));
	replaceAll(template_string, "<%vk_z%>", outputPointG2AffineAsHex(vk.rC_Z_g2, false));
	replaceAll(template_string, "<%vk_ic_length%>", std::to_string(icLength));

	std::stringstream ss;
	ss << "vk.IC[0] = Pairing.G1Point(" << outputPointG1AffineAsHex(vk.encoded_IC_query.first, false) << ");";
	for (size_t i = 1; i < icLength; ++i) {
		auto vkICi = outputPointG1AffineAsHex(vk.encoded_IC_query.rest.values[i - 1], false);
		ss << std::endl << "\t\tvk.IC[" << i << "] = Pairing.G1Point(" << vkICi << ");";
	}
	replaceAll(template_string, "<%vk_ic_pts%>", ss.str());
	replaceAll(template_string, "<%vk_input_length%>", std::to_string(icLength - 1));
	std::cout << template_string;
}

void export_proof(char *proof_fn) {
	libsnark::r1cs_ppzksnark_proof<libsnark::default_r1cs_ppzksnark_pp> proof;

	std::cout << "loading proof from file: " << proof_fn << std::endl;
	std::ifstream proof_file(proof_fn);
	if (!proof_file.good()) {
		std::cerr << "ERROR: " << proof_fn << " not found. " << std::endl;
		exit(1);
	}
	proof_file >> proof; 
	proof_file.close();

	std::cout << outputPointG1AffineAsHex(proof.g_A.g, true) << ", ";
	std::cout << outputPointG1AffineAsHex(proof.g_A.h, true) << ", ";
	std::cout << outputPointG2AffineAsHex(proof.g_B.g, true) << ", ";
	std::cout << outputPointG1AffineAsHex(proof.g_B.h, true) << ", ";
	std::cout << outputPointG1AffineAsHex(proof.g_C.g, true) << ", ";
	std::cout << outputPointG1AffineAsHex(proof.g_C.h, true) << ", ";
	std::cout << outputPointG1AffineAsHex(proof.g_H, true) << ", ";
	std::cout << outputPointG1AffineAsHex(proof.g_K, true) << std::endl;
}

void print_usage(char* argv[]) {
	std::cout << "usage: " << std::endl <<
		"(1) " << argv[0] << " contract <verification key file>" << std::endl <<
		"(2) " << argv[0] << " proof <proof file>" << std::endl;
}

int main (int argc, char* argv[]) {
	if (argc != 3) {
		print_usage(argv);
		exit(1);
	}
	libsnark::default_r1cs_ppzksnark_pp::init_public_params();
	if (!strcmp(argv[1], "contract")) {
		export_contract(argv[2]);
	} else if (!strcmp(argv[1], "proof")) {
		export_proof(argv[2]);
	} else {
		print_usage(argv);
		exit(1);
	}
}
