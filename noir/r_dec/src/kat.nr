// The KAT values of the R_dec tests (S13 U2). `cryptotool noir DIR`
// writes this text: the key share KEY = S G and the decryption share
// D = S C1 of C1 = C1_B1 of the R_ballot rows, with a fixed scalar S.
// Do not edit it: the gate compares it byte for byte.
// KEY_BAD = KEY + G. D_BAD = D + G.
pub global S: Field = 0x1756b31798aeb241bccc404198ee51fc3d7e8a92b4582d5813c13a1afd7a037f;
pub global KEY: [[u128; 3]; 2] = [
    [0x0899899670ea8075478ed254b9ae64, 0xcd4cdc4ae86c5a45c9fc3a32f8313b, 0x259b],
    [0x7298502c4b433784f22f9190e0f662, 0x7934111ffc084c0c2457ec73911a87, 0x1a58]
];
pub global C1: [[u128; 3]; 2] = [
    [0xb2796aaeadf92871e4ca355378cbc7, 0x620194426b06bf79f266002351e563, 0x1822],
    [0x9bb4aac83c8d7189a6434b2cbdf14f, 0xb4feaeb794f7fe88b0bcbb2ec15daf, 0x138c]
];
pub global D: [[u128; 3]; 2] = [
    [0x9b37581eb0ba2ed94514dea69c3f18, 0x5c6aab0349aedecb27f757d17a8653, 0x29b7],
    [0x31c1d86fabc37268180e16aaa2d13b, 0xc01563ed5a3f98a30a751ca2bff8d1, 0x0684]
];
pub global KEY_BAD: [[u128; 3]; 2] = [
    [0xa4df9ce7d3cc192ecab5aeed960986, 0x142326d5eb31948e4df625464e4e1f, 0x2351],
    [0x1622a260db1275c5c9251a2ee6f15e, 0x1f0d102c1f06506a617fb969ffda41, 0x143e]
];
pub global D_BAD: [[u128; 3]; 2] = [
    [0x0f563c2780a36f5fbb1b3245bcd2af, 0xfe59c554126428938b7625cf84c376, 0x0779],
    [0xe3a70593c6dfa384bbd566beac905b, 0xd3fcf34963122f4ba1ef88ee67d61d, 0x1e77]
];
