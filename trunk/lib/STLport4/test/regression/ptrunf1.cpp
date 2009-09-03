// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <iterator>
#include <functional>

#ifdef MAIN 
#define ptrunf1_test main
#endif
bool even(int n_)
{
  return(n_ % 2) == 0;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int ptrunf1_test(int, char**)
{
  cout<<"Results of ptrunf1_test:"<<endl;

int array [3] = { 1, 2, 3 };

  int* p = find_if((int*)array, (int*)array + 3, 
    pointer_to_unary_function<int, bool>(even));
  if(p != array + 3)
    cout << *p << " is even" << endl;
  return 0;
}
