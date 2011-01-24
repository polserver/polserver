// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define reverse1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int reverse1_test(int, char**)
{
  cout<<"Results of reverse1_test:"<<endl;
int numbers[6] = { 0, 1, 2, 3, 4, 5 };

  reverse(numbers, numbers + 6);
  for(int i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
