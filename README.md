# pepper_solidity_export

Script to export zkSnark proofs generated with pepper into solidity smart contracts.

# Pepper Installation

Install [pepper](https://github.com/pepper-project/pequin) to `~/pequin` (or chose another path and replace all occurrences in this tutorial).

To create a proof that is verifiable on the EVM, you need to replace the default curve `-DCURVE_BN128` with the EVM curve `-DCURVE_ALT_BN128` in `~/pequin/pepper/Makefile`.

Next, export pepper's libsnark installation folder into $LIBSNARK (our script shares the pepper's dependency instead of reinstalling it).
```sh
export LIBSNARK=~/pequin/thirdparty/libsnark/
```

# Exporting verification key into a smart contract

Assuming you have written a snark `myapp.c`. Inside `~/pequin/pepper` run:
```sh
./pepper_compile_and_setup_P.sh myapp
./pepper_compile_and_setup_V.sh myapp myapp.vkey myapp.pkey myapp.raw_vkey
```

The last argument `pepper_compile_and_setup_V` is important. This will export the unprocessed verification key, which is needed to create the smart contract.

Then inside the **pepper_solidity_export** repository run:

```sh
make export
bin/export contract ~/pequin/pepper/verification_material/myapp.raw_vkey
```

Deploy the generated smart contract e.g. using [Remix](https://ethereum.github.io/browser-solidity/)

# Verifying a pepper proof on the EVM

Inside `~/pequin/pepper` create a proof and also copy the inputs and outputs:
```sh
bin/pepper_prover_myapp prove myapp.pkey myapp.inputs myapp.outputs myapp.proof
cat prover_verifier_shared/myapp.inputs
cat prover_verifier_shared/myapp.outputs
```

Then inside the **pepper_solidity_export** repository run:
```sh
bin/export proof ~/pequin/pepper/prover_verifier_shared/myapp.proof
```

Amend the output of this script with another array containing **all** inputs and outputs you printed in the step above:

```
<exported proof>, [i_1, ..., i_n, o_1, ..., o_n]
```
Very large inputs/outputs might have to be converted into hex-strings. This can be easily done using python's `hex()` method.

Call `verifyTx` on your deployed smart contract with this list. You should see a transaction emitting a `Verified` event.
