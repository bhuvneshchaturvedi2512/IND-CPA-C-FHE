#include "seal/seal.h"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace seal;

uint64_t q = 132120577;

uint64_t pos_threshold = 64512;
uint64_t neg_threshold = 64512;

int error = 100;
int count_decryptions;

/*
Helper function: Convert a value into a hexadecimal string, e.g., uint64_t(17) --> "11".
*/
inline std::string uint64_to_hex_string(std::uint64_t value)
{
    return seal::util::uint_to_hex_string(&value, std::size_t(1));
}

int main()
{

    srand(967); //srand(967); srand(1073); srand(2512); srand(2406); srand(2022); for testing.
    
    EncryptionParameters parms(scheme_type::bfv);

    size_t poly_modulus_degree = 1024;
    parms.set_poly_modulus_degree(poly_modulus_degree);

    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));

    parms.set_plain_modulus(1024);

    SEALContext context(parms);

    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    
    uint64_t x;
    uint64_t y;

    Encryptor encryptor(context, secret_key);

    Evaluator evaluator(context);

    Decryptor decryptor(context, secret_key);
    
    Ciphertext x_encrypted;
    Ciphertext y_encrypted;
    
    Plaintext ans_original, ans_perturbed;
    	
    uint64_t ans_correct, ans_decrypted, ans_temp;

    FILE *count_of_oracle_accesses;
    
    uint64_t pos_min = 0;
    uint64_t pos_max = 64512;
    uint64_t neg_min = 0;
    uint64_t neg_max = 64512;
    
    int low, mid, high, distance_from_threshold;
    
    for(int i = 0; i < 1; ++i) 
    {
    
    	x = rand() % 256;
    	y = rand() % 256;
    	
    	Plaintext x_plain(uint64_to_hex_string(x));
    	Plaintext y_plain(uint64_to_hex_string(y));
    	
    	encryptor.encrypt(x_plain, x_encrypted);
    	encryptor.encrypt(y_plain, y_encrypted);
    	
    	evaluator.add_inplace(x_encrypted, y_encrypted);
    	
    	decryptor.decrypt(x_encrypted, ans_original);
    	
    	FILE* count_of_oracle_accesses = fopen("Number_of_decryptions.csv","a");
    	fprintf(count_of_oracle_accesses, "%d\n", 1);
    	fclose(count_of_oracle_accesses);
    	
    	for(int j = 0; j < poly_modulus_degree; ++j)
    	{
    	    count_decryptions = 0;
    	    
    	    ans_correct = ans_original.data()[j];
    	    
    	    distance_from_threshold = pos_threshold - pos_min;
    	    
    	    x_encrypted.data()[j] = (x_encrypted.data()[j] + distance_from_threshold) % q;
    	    
    	    decryptor.decrypt(x_encrypted, ans_perturbed);
    	    
    	    x_encrypted.data()[j] = (x_encrypted.data()[j] - distance_from_threshold) % q;
    	    
    	    ans_decrypted = ans_perturbed.data()[j];
    	    
    	    count_decryptions = count_decryptions + 1;
    	    
    	    distance_from_threshold = neg_threshold - neg_min;
    	    
    	    x_encrypted.data()[j] = (x_encrypted.data()[j] - distance_from_threshold) % q;
    	    
    	    decryptor.decrypt(x_encrypted, ans_perturbed);
    	    
    	    x_encrypted.data()[j] = (x_encrypted.data()[j] + distance_from_threshold) % q;
    	    
    	    ans_temp = ans_perturbed.data()[j];
    	    
    	    count_decryptions = count_decryptions + 1;
    	    
    	    FILE* m_coeffs = fopen("m_coeffs.txt","a");
    	    fprintf(m_coeffs, "%ld ", ans_correct);
    	    fclose(m_coeffs);
    	    
    	    FILE* b_coeffs = fopen("b_coeffs.txt","a");
    	    fprintf(b_coeffs, "%ld ", x_encrypted.data()[j]);
    	    fclose(b_coeffs);
    	    
    	    FILE* a_coeffs = fopen("a_coeffs.txt","a");
    	    fprintf(a_coeffs, "%ld ", x_encrypted.data(1)[j]);
    	    fclose(a_coeffs);
    	    
    	    if(ans_decrypted == ans_temp)
    	    {
    	        FILE* recovered_errors = fopen("recovered_errors.txt","a");
    	        fprintf(recovered_errors, "%d ", 0);
    	        fclose(recovered_errors);
    	        
    	        FILE* count_of_oracle_accesses = fopen("Number_of_decryptions.csv","a");
    	        fprintf(count_of_oracle_accesses, "%d\n", count_decryptions);
    	        fclose(count_of_oracle_accesses);
    	        
    	        continue;
    	    }
    	    
    	    //b = -(as + e) = -as + -e. When e1 + e2 > 0, e < 0, and vice versa.
    	    if(ans_decrypted != ans_correct)//Error is positive
    	    {
    	        low = pos_min;
    	        high = pos_max;
    	        
    	        while(low <= high)
    	        {
    	            mid = (low + high)/2;
    	            
    	            if(low == high - 1)
    	            {
    	                break;
    	            }
    	            
    	            distance_from_threshold = pos_threshold - mid;
    	            
    	    	    x_encrypted.data()[j] = (x_encrypted.data()[j] + distance_from_threshold) % q;
    	    
    	    	    decryptor.decrypt(x_encrypted, ans_perturbed);
    	    
    	    	    x_encrypted.data()[j] = (x_encrypted.data()[j] - distance_from_threshold) % q;
    	    
    	    	    ans_decrypted = ans_perturbed.data()[j];
    	    
    	    	    count_decryptions = count_decryptions + 1;
    	    	    
    	    	    if(ans_decrypted == ans_correct)
    	    	    {
    	    	        error = mid;
    	    	        high = mid;
    	    	    }
    	    	    else
    	    	    {
    	    	        low = mid;
    	    	    }
    	    
    	        }
    	    }
    	    else//Error is negative
    	    {
    	        low = neg_min;
    	        high = neg_max;
    	        
    	        while(low <= high)
    	        {
    	            mid = (low + high)/2;
    	            
    	            if(low == high - 1)
    	            {
    	                break;
    	            }
    	            
    	            distance_from_threshold = neg_threshold - mid;
    	            
    	    	    x_encrypted.data()[j] = (x_encrypted.data()[j] - distance_from_threshold) % q;
    	    
    	    	    decryptor.decrypt(x_encrypted, ans_perturbed);
    	    
    	    	    x_encrypted.data()[j] = (x_encrypted.data()[j] + distance_from_threshold) % q;
    	    
    	    	    ans_decrypted = ans_perturbed.data()[j];
    	    
    	    	    count_decryptions = count_decryptions + 1;
    	    	    
    	    	    if(ans_decrypted == ans_correct)
    	    	    {
    	    	        error = -mid;
    	    	        high = mid;
    	    	    }
    	    	    else
    	    	    {
    	    	        low = mid;
    	    	    }
    	    
    	        }
    	    }
    	    
    	    FILE* recovered_errors = fopen("recovered_errors.txt","a");
    	    fprintf(recovered_errors, "%d ", error);
    	    fclose(recovered_errors);
    	    
    	    FILE* count_of_oracle_accesses = fopen("Number_of_decryptions.csv","a");
    	    fprintf(count_of_oracle_accesses, "%d\n", count_decryptions);
    	    fclose(count_of_oracle_accesses);
    	}
	
    }
    cout << "All errors recovered!!\n" << endl;

}
