// The KAT values of the R_ballot tests (S13 U2). `cryptotool noir DIR`
// writes this text: the T1 ElGamal of the bits 0, 1 and 2 under one key
// PK, with fixed scalars. Do not edit it: the gate compares it byte for
// byte. A point is [x, y]; a coordinate is the 3 limbs of a BN254_Fq.
// C2_B2 = 2 G + R1 PK. C1_BAD = C1_B1 + G. C2_BAD = C2_B1 + PK.
pub global R0: Field = 0x0d43608bdd32191dcee81c3166ccbbf836261878e6f555d143347c0360878512;
pub global R1: Field = 0x1863f329a3fbad9107495e535a6992e4d40c2c9db09f373c307ea07187813a9a;
pub global CTX: [u8; 32] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31];
pub global PK: [[u128; 3]; 2] = [
    [0xef5293e4059daa0768d1548ff33191, 0xda5f3cebed9f06b48a0d7dd69c8b0d, 0x1eaf],
    [0x0ddd01599155a531604cba456683b0, 0xbe1d4f0127d7b59d9f59fc4f290b87, 0x2ae3]
];
pub global C1_B0: [[u128; 3]; 2] = [
    [0x6b22b1f5ecf262f018b48f0b02494a, 0x4dd26cf9673bd520c924ae5140f259, 0x0894],
    [0x69bff6e809dec6fab290e8673decfa, 0x0183d31906d2c75a29c8c43fb1c9d5, 0x2171]
];
pub global C2_B0: [[u128; 3]; 2] = [
    [0xc43826b2b22730abd4e2e854eaf0a0, 0x8ab815e9dcf32ee70a9a85fea4fa9d, 0x2b05],
    [0xcef602157141c163b10dcebd8db79c, 0x9b94fec3b35dcee4212703ec669d20, 0x2c36]
];
pub global C1_B1: [[u128; 3]; 2] = [
    [0xb2796aaeadf92871e4ca355378cbc7, 0x620194426b06bf79f266002351e563, 0x1822],
    [0x9bb4aac83c8d7189a6434b2cbdf14f, 0xb4feaeb794f7fe88b0bcbb2ec15daf, 0x138c]
];
pub global C2_B1: [[u128; 3]; 2] = [
    [0x7bb06b9c231bd7d870af789b1ef6e1, 0xcef2227821b2ebd56fa0066115d195, 0x03ee],
    [0xd675fdb6507fbdaa14ff3a9fe11586, 0xad2ee3b53d2951399ba0c7d273cf06, 0x1ec6]
];
pub global C2_B2: [[u128; 3]; 2] = [
    [0xb660370689d9bf97b12e67c5d2647a, 0x31197ed7e78cded4f6d12c6bae0fef, 0x171b],
    [0xa4b13fee240e74bca2d6d0a96ec7ef, 0xd93e53f9afd6f10d314dc3d49db74a, 0x2140]
];
pub global C1_BAD: [[u128; 3]; 2] = [
    [0x3216d8bf1aef9b706462ab5e2f9d6e, 0x1a78185e8c526c893ac309f8efdfc6, 0x2623],
    [0x3ab1bfc3acd49becf0eb2dff53a242, 0x5aef8a4a06be44e333cf7a2dae1bae, 0x1097]
];
pub global C2_BAD: [[u128; 3]; 2] = [
    [0x5af372120f28fcd81394a4942d2915, 0x0df0ade44732b24295857d60b82c29, 0x0100],
    [0xe2f1dcab7c5b2c4eaa62e2f21d297c, 0xd4791527810bdf5a9cbe96f9c19768, 0x15e6]
];
