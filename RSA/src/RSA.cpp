#include "RSA.h"

RSA::RSA(){
  big_int p, q;
  big_int phi_n;
  //create threads for faster computation
  // thread 1
  std::promise<big_int> promisedP;
  std::future<big_int> futureP = promisedP.get_future();
  std::thread th1(&RSA::GeneratePrime, this, &promisedP);
    
  do {
    do
      q = GenerateNum(1024); 
    while (q % 2 == 0);
  } while (!PrimalityTest(20, q));
  p = futureP.get();
  // Waiting for thread to finish
  th1.join();
  modulo = p * q;
  phi_n = (p - 1) * (q - 1);
  do
    publicKey = GenerateNum(2048) % (phi_n - 2) + 2; // 1 < publicKey < phi_n
  while (GCD(publicKey, phi_n) != 1);

  privateKey = ExtendedGCD(phi_n, publicKey);
}

big_int RSA::GetPublicKey(){
  return publicKey;
}

big_int RSA::GetModulo(){
  return modulo;
}

void RSA::GeneratePrime(std::promise<big_int> * promObj){
  big_int num;
  do {
    do
      num = GenerateNum(1024);
    while (num % 2 == 0);
  } while (!PrimalityTest(20, num));
  promObj->set_value(num);
}

big_int RSA::GenerateNum(int NumOfBits){
  big_int Num;
  int NumOfDigits = NumOfBits/3.321928095 + 1;
  int temp;
  std::stringstream ss;
  temp = rand()%9+1; //Make sure the first number is not 0
  ss << temp;
  for (int i = 0; i < NumOfDigits-1; i++)
  {
    temp = rand()%10;
    ss << temp;
  }
  ss >> Num;
  return Num;
}

bool RSA::PrimalityTest(int IterNum, big_int n){
  int primes[] = {3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,
  79,83,89,97,101,103,107,109,113,131,137,139,149,151,157,163,167,173,179,
  181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,
  281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,
  397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541};
  for (int i = 0; i < 98; i++)
  {
    if (n % primes[i]==0)
      return false;
  }

  for (int j = 0; j < IterNum; ++j){
    big_int d = n - 1;
    big_int k = 0;
    big_int T;
    // select a random base
    big_int a = 2 + rand() % (n - 4); 

    // Find d,k such that n = (2^k)*d + 1
    while (d % 2 == 0) {
      k++;
      d = d / 2;
    }

    // Miller-Rabin test
    T = ModularExponentiation(a, d, n);

    if (T == 1 || T == n - 1)
      return true;

    for (int i = 0; i < k; i++) {
      T = ModularExponentiation(T, 2, n);
      if (T == 1)
        return false;
      if (T == n - 1)
        return true;
    }
  }
  return false;
}

big_int RSA::ModularExponentiation(big_int base, big_int exponent, big_int modulus){
  big_int r;
  big_int y = 1;

//Right-to-left binary method
  while (exponent > 0) {
    r = exponent % 2;
    if (r)
      y = y * base % modulus;
    base = base * base % modulus;
    exponent = exponent>>1;
  }
  return y;
}

big_int RSA::GCD(big_int a, big_int b)
{
  big_int temp;

  if (a < b) {
    temp = a;
    a = b;
    b = temp;
  }

  while (b > 0) {
    temp = a % b;
    a = b;
    b = temp;
  }

  return a;
}

big_int RSA::ExtendedGCD(big_int a, big_int b){
  //ax + by = gcd(a,b)
  big_int inv, x = 0, y = 1;
  big_int q, temp, tempA = a;

  while (b > 0) {
    q = a / b; //Do not remove q
    temp = a % b;
    a = b;
    b = temp;

    temp = x - q * y;
    x = y;
    y = temp;
  }

  if (a == 1)
    inv = x;

  if (inv < 0)
    inv = inv + tempA;

  return inv;
}

void RSA::Encryption(std::ifstream& inp, std::string plainFile,
	std::ofstream& out, std::string cipherFile){
  inp.open(plainFile);
  // destroy contents of these files (from previous runs, if any)
  out.open(cipherFile);
  out.close();
  // Check if files can be opened.
  if (!inp) {
    printf("Error opening Source File.\n");
    exit(1);
  }

  out.open(cipherFile);
  if (!out) {
    printf("Error opening Destination File.\n");
    exit(1);
  }
  
  while (true) {
    char ch;
    inp.read(&ch,1);
    if (inp.eof())
      break;
    int value = toascii(ch);
    big_int cipher;
    cipher = ModularExponentiation(value,publicKey , modulo);
    out << cipher << " ";
  }
  
  inp.close();
  out.close();
}

void RSA::EncryptionWithForeignKey(std::ifstream& inp, std::string plainFile, std::ofstream& out,
		std::string cipherFile, big_int ForeignModulo, big_int ForeignPublicKey) {
  inp.open(plainFile);
  // destroy contents of these files (from previous runs, if any)
  out.open(cipherFile);
  out.close();
  // Check if files can be opened.
  if (!inp) {
    printf("Error opening Source File.\n");
    exit(1);
  }

  out.open(cipherFile);
  if (!out) {
    printf("Error opening Destination File.\n");
    exit(1);
  }

  while (true) {
    char ch;
    inp.read(&ch, 1);
    if (inp.eof())
      break;
    int value = toascii(ch);
    big_int cipher;
    cipher = ModularExponentiation(value, ForeignPublicKey, ForeignModulo);
    out << cipher << " ";
  }

  inp.close();
  out.close();
}

void RSA::Decryption(std::ifstream& inp, std::string cipherFile, 
	std::ofstream& out, std::string decipherFile){
  inp.open(cipherFile);
  // destroy contents of these files (from previous runs, if any)
  out.open(decipherFile);
  out.close();

  // Check if files can be opened.
  if (!inp) {
    std::cout << "Error opening Cipher Text.\n";
    exit(1);
  }

  out.open(decipherFile);
  if (!out) {
    std::cout << "Error opening File.\n";
    exit(1);
  }
  
  while (1) {
    big_int cipherNum;
    std::stringstream ss;
    big_int decipher;
    int temp;
    
    if (!(inp >> cipherNum))
      break;

    decipher = ModularExponentiation(cipherNum, privateKey, modulo);
    ss << decipher;
    ss >> temp;
    out << (char)temp;
  }
  out.close();
  inp.close();
}
