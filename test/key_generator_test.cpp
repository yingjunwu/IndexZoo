// #include <map>
// #include <unordered_map>
// #include <vector>

// #include "harness.h"

// #include "key_generator_all.h"


// class KeyGeneratorTest : public LearnedIndexTest {};


// TEST_F(KeyGeneratorTest, Uint64Test) {

//   const int nrolls=10000;  // number of experiments
//   const int nstars=100;    // maximum number of stars to distribute

//   std::default_random_engine generator;
//   std::normal_distribution<double> distribution(100.0,2.0);

//   int p[100]={};

//   for (int i=0; i<nrolls; ++i) {
//     double number = distribution(generator);
//     if ((number>=0.0)&&(number<10.0)) ++p[int(number)];
//   }

//   std::cout << "normal_distribution (5.0,2.0):" << std::endl;

//   for (int i=0; i<100; ++i) {
//     std::cout << i << "-" << (i+1) << ": ";
//     std::cout << std::string(p[i]*nstars/nrolls,'*') << std::endl;
//   }


// }

