// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iterator>

#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <numeric>
// #include <functional>

#ifdef MAIN 
#define setsymd1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int setsymd1_test(int, char**)
{
  cout<<"Results of setsymd1_test:"<<endl;
  vector <int> v1(10);
  iota(v1.begin(), v1.end(), 0);
  vector <int> v2(10);
  iota(v2.begin(), v2.end(), 7);
  ostream_iterator <int> iter(cout, " ");
  cout << "v1: ";
  copy(v1.begin(), v1.end(), iter);
  cout << "\nv2: ";
  copy(v2.begin(), v2.end(), iter);
  cout << endl;
  set_symmetric_difference(v1.begin(), v1.end(), 
                           v2.begin(), v2.end(), iter);
  cout << endl;
  return 0;
}
