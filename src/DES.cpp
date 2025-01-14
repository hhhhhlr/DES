//
// Created by OneCarrot on 2019-04-25.
//

#include "DES.h"
#include "cassert"


//-----转换表-----
map<char, int> charToInt = {{'0', 0}, {'1', 1}};
map<int, char> intToChar = {{1,'1'}, {'0', 0}};

map<int, string> intToBinary = {
        {0,"0000"}, {1,"0001"}, {2,"0010"}, {3,"0011"}, {4,"0100"}, {5,"0101"}, {6,"0110"}, {7,"0111"},
        {8,"1000"}, {9,"1001"}, {10,"1010"}, {11,"1011"}, {12,"1100"}, {13,"1101"}, {14,"1110"}, {15,"1111"}
};


DES::DES(string K) {
    this->K = BigInteger(K).toBinary(0);
    generateAllKi();
    //generateKifirstRound();
}

DES::DES(string K, string target, Direction initState) {

    this->K = BigInteger(K).toBinary(0);
    if(initState == ENCRPT) {
        this->plaintext = BigInteger(target).toBinary(0);
    } else {
        this->cipher = BigInteger(target).toBinary(0);
    }
    generateAllKi();
    //generateKifirstRound();

}

DES::DES(BigInteger K, BigInteger target, Direction initState) {
    this->K = K.toBinary(0);
    if(initState == ENCRPT) {
        this->plaintext = target.toBinary(0);
    } else {
        this->cipher = target.toBinary(0);
    }
    generateAllKi();
    //generateKifirstRound();
}

BigInteger DES::InitialPermutation(BigInteger plain) {

    return universalPermutation(plain, IP);

}

void DES::generateKifirstRound() {

    stringstream tempstream;


    BigInteger temp = universalPermutation(K, PC_1);


    C = splitBit(temp, 28, LEFT);
    D = splitBit(temp, 28, RIGHT);


}

BigInteger DES::generateKi(BigInteger C, BigInteger D, int round) {

    int LSi = LS[round];
    stringstream cd;
    C.cyclicShift(LSi, LEFT);
    D.cyclicShift(LSi, LEFT);
    cd<<C.toString() << D.toString();
    BigInteger CD(cd.str());

    this->C = splitBit(CD, 28, LEFT);
    this->D = splitBit(CD, 28, RIGHT);

    return universalPermutation(CD, PC_2);

}

void DES::generateAllKi() {

    generateKifirstRound();
    for(int i = 0; i < 16; i++) {
        Ki[i] = generateKi(C, D, i+1);
    }

}

BigInteger DES::splitBit(BigInteger bits, int size, bool side) {

    stringstream lstream;
    stringstream rstream;

    for(int i = 0; i < bits.getSize(); i++) {
        if( i<  size )
            rstream << bits.valueOf(size - i - 1);
        if( i>= size )
            lstream << bits.valueOf(bits.getSize() - i + size - 1);
    }

    if(!side)
        return BigInteger(lstream.str());
    else
        return BigInteger(rstream.str());
}


BigInteger DES::Expansion(BigInteger R) {

    return universalPermutation(R, E);

}

BigInteger DES::Substitution(BigInteger var) {

    stringstream resultss;

    for(int i = 0, j = 6; i < 8; i++ ){
        int base = j * (8 - i - 1);
        int row = charToInt[var.valueOf(base)] + charToInt[var.valueOf(base+5)] *2;
        int column = charToInt[var.valueOf(base+1)] + charToInt[var.valueOf(base+2)]*2 +
                        charToInt[var.valueOf(base+3)]*4 + charToInt[var.valueOf(base+4)]*8;
        int result = S[i][row][column];

        resultss <<  intToBinary[result];
    }

    return BigInteger(resultss.str());

}

BigInteger DES::Permutation(BigInteger var) {

    return universalPermutation(var, P);

}

BigInteger DES::FeistelFunc(BigInteger K, BigInteger R) {

    BigInteger ex = Expansion(R);
    BigInteger xorrr = K.XOR(ex);

     return Permutation(Substitution(K.XOR(Expansion(R))));

}

BigInteger DES::RoundFunc(BigInteger var, BigInteger Ki, int round) {

    BigInteger L = splitBit(var, var.getSize()/2, LEFT);
    BigInteger R = splitBit(var, var.getSize()/2, RIGHT);

    BigInteger resultL = R;
    BigInteger resultR = L.XOR(FeistelFunc(Ki, R));

    stringstream ss;
    ss << resultL.toString() << resultR.toString();

    return BigInteger(ss.str());

}

BigInteger DES::Round16(BigInteger var, Direction d) {

    BigInteger tempRoundRes = var; //初始置换
    if(d == ENCRPT) {

        for(int i = 0; i < 16; i++){

            //BigInteger Ki = generateKi(C, D, i+1);

            BigInteger oneKi = Ki[i];

            tempRoundRes = RoundFunc(tempRoundRes, oneKi, i+1);
        }
    } else {

        for(int i = 15; i >= 0; i--) {

            BigInteger oneKi = Ki[i];
            tempRoundRes = RoundFunc(tempRoundRes, oneKi, i+1);
        }

    }

    return tempRoundRes;

}

BigInteger DES::ReversePosition(BigInteger var) {

    BigInteger L = splitBit(var, var.getSize()/2, LEFT);
    BigInteger R = splitBit(var, var.getSize()/2, RIGHT);

    stringstream ss;
    ss << R.toString() << L.toString();

    return BigInteger(ss.str());

}

BigInteger DES::InitialInversePermutation(BigInteger var) {

    return universalPermutation(var, inverseIP);

}


void DES::Encrypt() {

    BigInteger initpermutation = InitialPermutation(plaintext);
    BigInteger round16 = Round16(initpermutation, ENCRPT);
    BigInteger reverspos = ReversePosition(round16);
    BigInteger initInvPer = InitialInversePermutation(reverspos);

    this->cipher = initInvPer;

}

void DES::Decrypt() {

    BigInteger initpermutation = InitialPermutation(cipher);
    BigInteger round16 = Round16(initpermutation, DECRPT);
    BigInteger reverspos = ReversePosition(round16);
    BigInteger initInvPer = InitialInversePermutation(reverspos);

    this->plaintext = initInvPer;

}





//----私有成员setter/getter----
void DES::setPlaintext(string plaintext) {
    this->plaintext = BigInteger(plaintext).toBinary(0);
}

void DES::setPlaintext(BigInteger plaintext) {
    this->plaintext = plaintext.toBinary(0);
}

void DES::setCipher(string cipher) {
    this->cipher = BigInteger(cipher).toBinary(0);
}

void DES::setCipher(BigInteger cipher) {
    this->cipher = cipher.toBinary(0);
}

void DES::setKey(string key) {
    this->K = BigInteger(key).toBinary(0);
}

void DES::setKey(BigInteger key) {
    this->K = key.toBinary(0);
}

void DES::setState(Direction direction) {
    this->initState = direction;
}

string DES::getCipher() {
    return cipher.toHex().toString();
}

string DES::getPlaintext() {
    return plaintext.toHex().toString();
}




//----私有函数定义----

BigInteger DES::universalPermutation(BigInteger var, map<int, int> perMap) {

    //assert(var.getSize() == perMap.size());

    stringstream res;
    for(int i = 0; i < perMap.size(); i++) {
        res << var.valueOf(var.getSize() - perMap[i+1]);
    }

    return BigInteger(res.str());
}





//----------加密置换表-------------

map<int, int> DES::IP = { {1,58}, {2,50}, {3,42}, {4,34}, {5,26}, {6,18}, {7,10}, {8,2},
                          {9,60},{10,52},{11,44},{12,36},{13,28},{14,20},{15,12},{16,4},
                          {17,62},{18,54},{19,46},{20,38},{21,30},{22,22},{23,14},{24,6},
                          {25,64},{26,56},{27,48},{28,40},{29,32},{30,24},{31,16},{32,8},
                          {33,57},{34,49},{35,41},{36,33},{37,25},{38,17}, {39,9},{40,1},
                          {41,59},{42,51},{43,43},{44,35},{45,27},{46,19},{47,11},{48,3},
                          {49,61},{50,53},{51,45},{52,37},{53,29},{54,21},{55,13},{56,5},
                          {57,63},{58,55},{59,47},{60,39},{61,31},{62,23},{63,15},{64,7}};

map<int, int> DES::inverseIP = { {1,40}, {2,8}, {3,48}, {4,16}, {5,56}, {6,24}, {7,64}, {8,32},
                                 {9,39},{10,7},{11,47},{12,15},{13,55},{14,23},{15,63},{16,31},
                                 {17,38},{18,6},{19,46},{20,14},{21,54},{22,22},{23,62},{24,30},
                                 {25,37},{26,5},{27,45},{28,13},{29,53},{30,21},{31,61},{32,29},
                                 {33,36},{34,4},{35,44},{36,12},{37,52},{38,20},{39,60},{40,28},
                                 {41,35},{42,3},{43,43},{44,11},{45,51},{46,19},{47,59},{48,27},
                                 {49,34},{50,2},{51,42},{52,10},{53,50},{54,18},{55,58},{56,26},
                                 {57,33},{58,1},{59,41}, {60,9},{61,49},{62,17},{63,57},{64,25}};

map<int,int> DES::E = { {1,32},  {2,1},  {3,2},  {4,3},  {5,4},  {6,5},
                        {7,4},  {8,5},  {9,6}, {10,7}, {11,8}, {12,9},
                        {13,8}, {14,9},{15,10},{16,11},{17,12},{18,13},
                        {19,12},{20,13},{21,14},{22,15},{23,16},{24,17},
                        {25,16},{26,17},{27,18},{28,19},{29,20},{30,21},
                        {31,20},{32,21},{33,22},{34,23},{35,24},{36,25},
                        {37,24},{38,25},{39,26},{40,27},{41,28},{42,29},
                        {43,28},{44,29},{45,30},{46,31},{47,32},{48,1}};

int DES::S[8][4][16] = {  {{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
                           {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
                           {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
                           {15,12,8,2,4,9,1,7,5,11 ,3,14,10,0,6,13}},

                          {{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
                           {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
                           {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
                           {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}},

                          {{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
                           {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
                           {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
                           {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}},

                          {{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
                           {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
                           {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
                           {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}},

                          {{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
                           {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
                           {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
                           {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}},

                          {{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
                           {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
                           {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
                           {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}},

                          {{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
                           {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
                           {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
                           {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}},

                          {{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
                           {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
                           {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
                           {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}}};


map<int, int> DES::P = { {1,16},  {2,7}, {3,20}, {4,21}, {5,29}, {6,12}, {7,28}, {8,17},
                          {9,1},{10,15},{11,23},{12,26}, {13,5},{14,18},{15,31},{16,10},
                         {17,2}, {18,8},{19,24},{20,14},{21,32},{22,27}, {23,3}, {24,9},
                        {25,19},{26,13},{27,30}, {28,6},{29,22},{30,11}, {31,4},{32,25}};


map<int, int> DES::PC_1 = {  {1,57}, {2,49}, {3,41}, {4,33}, {5,25}, {6,17},  {7,9},
                              {8,1}, {9,58},{10,50},{11,42},{12,34},{13,26},{14,18},
                            {15,10}, {16,2},{17,59},{18,51},{19,43},{20,35},{21,27},
                            {22,19},{23,11}, {24,3},{25,60},{26,52},{27,44},{28,36},
                            {29,63},{30,55},{31,47},{32,39},{33,31},{34,23},{35,15},
                             {36,7},{37,62},{38,54},{39,46},{40,38},{41,30},{42,22},
                            {43,14}, {44,6},{45,61},{46,53},{47,45},{48,37},{49,29},
                            {50,21},{51,13}, {52,5},{53,28},{54,20},{55,12},{56,4}};

map<int, int> DES::PC_2 = { {1,14},{2,17},{3,11},{4,24},{5,1},{6,5},{7,3},{8,28},
                            {9,15},{10,6},{11,21},{12,10},{13,23},{14,19},{15,12},{16,4},
                            {17,26},{18,8},{19,16},{20,7},{21,27},{22,20},{23,13},{24,2},
                            {25,41},{26,52},{27,31},{28,37},{29,47},{30,55},{31,30},{32,40},
                            {33,51},{34,45},{35,33},{36,48},{37,44},{38,49},{39,39},{40,56},
                            {41,34},{42,53},{43,46},{44,42},{45,50},{46,36},{47,29},{48,32}};

map<int, int> DES::LS = {{1,1},{2,1},{3,2},{4,2},{5,2},{6,2},{7,2},{8,2},
                         {9,1},{10,2},{11,2},{12,2},{13,2},{14,2},{15,2},{16,1}};
