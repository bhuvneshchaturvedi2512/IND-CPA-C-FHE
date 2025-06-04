#include <iostream>
#include <cstdlib>
#include<string.h>
#include<stdlib.h>
#include "LWE.h"
#include "FHEW.h"
#include "distrib.h"

using namespace std;

void help(char* cmd) {
  cerr << "\nusage: " << cmd << " n\n\n" 
  << "  Generate a secret key sk and evaluation key ek, and repeat the following test n times:\n"
  << "   - generate random bits b1,b2,b3,b4\n"
  << "   - compute ciphertexts c1, c2, c3 and c4 encrypting b1, b2, b3 and b4  under sk\n"
  << "   - homomorphically compute the encrypted (c1 NAND c2) NAND (c3 NAND c4) \n"
  << "   - decrypt all the intermediate results and check correctness \n"
  << "\n If any of the tests fails, print ERROR and stop immediately.\n\n";
  exit(0);
}

int cleartext_gate(int v1, int v2, BinGate gate){
  switch(gate)
  {
    case OR: return v1 || v2;
    case AND: return v1 && v2;
    case NOR: return not(v1 || v2);
    case NAND: return not(v1 && v2);
    default: cerr << "\n This gate does not exists \n"; exit(1); return 0;
  }
}

void cerr_gate(BinGate gate){
  switch(gate)
  {
    case OR: cerr << " OR\t"; return;
    case AND: cerr << " AND\t"; return;
    case NOR: cerr << " NOR\t"; return;
    case NAND: cerr << " NAND\t"; return;
  }
}

int pos_threshold = 63;
int neg_threshold = 64;

int error = 100;
int count;

void get_error_if_positive(LWE::CipherText& e12, int start, int end, LWE::SecretKey& key, int ans) {

    int mid = (start + end)/2;
   
    int ci, distance_from_threshold;

    if(start == end-1) { //base case for recursion
        return;
    }
    
    distance_from_threshold = pos_threshold - mid;
    
    e12.b = (e12.b + distance_from_threshold) % q;
    
    ci = LWE::Decrypt(key, e12);
    count = count + 1;
    
    e12.b = (e12.b - distance_from_threshold) % q;

    if(ci == ans) {//if decrypted correctly
    	error = mid;
    	get_error_if_positive(e12, start, mid, key, ans);
    }
    else {
    	get_error_if_positive(e12, mid, end, key, ans);
    }
    
    //return 100;
}

void get_error_if_negative(LWE::CipherText& e12, int start, int end, LWE::SecretKey& key, int ans) {

    int mid = (start + end)/2;
   
    int ci, distance_from_threshold;

    if(start == end-1) { //base case for recursion
        return;
    }
    
    distance_from_threshold = neg_threshold - mid;
    
    e12.b = (e12.b - distance_from_threshold) % q;
    
    ci = LWE::Decrypt(key, e12);
    count = count + 1;
    
    e12.b = (e12.b + distance_from_threshold) % q;

    if(ci == ans) {//if decrypted correctly
    	error = -mid;
    	get_error_if_negative(e12, start, mid, key, ans);
    }
    else {
    	get_error_if_negative(e12, mid, end, key, ans);
    }
    
    //return 100;
}

void get_error(LWE::CipherText& e12, int pos_min, int pos_max, int neg_min, int neg_max, LWE::SecretKey& key, int ans) {

    int distance_from_threshold = pos_threshold - pos_min;
    
    e12.b = (e12.b + distance_from_threshold) % q;
    
    int ai = LWE::Decrypt(key, e12);
    count = count + 1;

    e12.b = (e12.b - distance_from_threshold) % q;
    
    //int error_1 = 0;
    
    if(ans != ai) {

    	get_error_if_positive(e12, pos_min, pos_max, key, ans);
    	
    	if(error != -1) //If the recovered error is one hundred, the sample will be rejected as such an error value is out of bounds, implying some error has occured
    	{
    		FILE* gaussian_elimination = fopen("gaussian_elimination.txt","a");
    		fprintf(gaussian_elimination, "recovered_error: %d m: %d b: %d a: ", error, ans == 1 ? 128 : 0, e12.b);
    		for (int32_t i = 0; i < n; ++i)
    		{
    			fprintf(gaussian_elimination, "%d,", e12.a[i]);
    		}
    		fprintf(gaussian_elimination, "\n");
    		fclose(gaussian_elimination);
    	}
    	
    }
    else {
    
	get_error_if_negative(e12, neg_min, neg_max, key, ans);
    	
    	if(error != -1) //If the recovered error is one hundred, the sample will be rejected as such an error value is out of bounds, implying some error has occured
    	{
    		FILE* gaussian_elimination = fopen("gaussian_elimination.txt","a");
    		fprintf(gaussian_elimination, "recovered_error: %d m: %d b: %d a: ", error, ans == 1 ? 128 : 0, e12.b);
    		for (int32_t i = 0; i < n; ++i)
    		{
    			fprintf(gaussian_elimination, "%d,", e12.a[i]);
    		}
    		fprintf(gaussian_elimination, "\n");
    		fclose(gaussian_elimination);
    	}
    }
    
    FILE* count_of_oracle_accesses = fopen("Number_of_decryptions.csv","a");
    fprintf(count_of_oracle_accesses, "%d\n", count);
    fclose(count_of_oracle_accesses);
    
}


int main(int argc, char *argv[]) {

  srand(1073);

  cerr << "Setting up FHEW \n";
  FHEW::Setup();
  cerr << "Generating secret key ... ";
  LWE::SecretKey LWEsk;
  LWE::KeyGen(LWEsk);
  
  FILE *secret_key = fopen("secret_key.txt","a");
  for (int i = 0; i < n; ++i) {
    fprintf(secret_key, "%d ", LWEsk[i]);
  }
  fprintf(secret_key, "\n");
  fclose(secret_key);
  
  cerr << " Done.\n";
  cerr << "Generating evaluation key ... this may take a while ... ";
  FHEW::EvalKey EK;
  FHEW::KeyGen(&EK, LWEsk);
  cerr << " Done.\n\n";

  int v1, v2, ans;
  
  LWE::CipherText e1, e2, e12;
    
  int pos_min = 0;
  int pos_max = 63;
  int neg_min = 0;
  int neg_max = 64;
  
  BinGate gate = static_cast<BinGate>(3);

  for(int i = 1; i <= 650; i++) {
  
    v1 = rand()%2;
    v2 = rand()%2;
    
    cerr << "Running iteration " << i << endl;
    
    LWE::Encrypt(&e1, LWEsk, v1);
    LWE::Encrypt(&e2, LWEsk, v2);
    
    FHEW::HomGate(&e12, gate, EK, e1, e2);
    
    count = 0;

    ans = LWE::Decrypt(LWEsk, e12);
    count = count + 1;

    get_error(e12, pos_min, pos_max, neg_min, neg_max, LWEsk, ans);
    
  }
  
}


