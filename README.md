# visp-codec
VISP (Vector Integer State Prediction). Lossless data codec.

This is a codec for any kind of data encoding with no loss. The codec is based on the method of the sequence of bits representation in the form of an arbitrary length vector with the process of Gram- Schmidt orthogonalisation being applied. The combination of several practices of ” unfolding “ the numerical sequence allows to reduce the entropy of despairingly complex sequences, which have not been able to be encoded until now.

We called this process as “a rectification”. Then a combination of “weak fractions” calculation with the help of a predictor is applied to this “expanded” space of the source data. Each iteration corresponds to the principle of a rigid determinism. All the operations are integral and can be determined with the help of the state table. To improve the quality of the coding at the stage of “rarefaction”, different methods of an arithmetic coding are applied to the initial data.

Our codec is an enclosed software. We believe that we have found an opportunity to improve it for a long time more, due to the fact, that the theory for the process, we’ve found, is not entirely explored. Our team is working hard for the public could start using our software for their own purposes in the nearest future without any restrictions.

You are able to use our software without any restrictions for a personal, non-commercial use. The products, that are going to be created on the basis of our SDK, will be committed by the patent law and represented to the community for reviewing and getting the possibility of acquiring the license law for a commercial use in their products or for the needs of the corporate customers.
