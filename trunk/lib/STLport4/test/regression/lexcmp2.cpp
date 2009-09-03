// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <functional>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define lexcmp2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int lexcmp2_test(int, char**)
{
  cout<<"Results of lexcmp2_test:"<<endl;
const unsigned size = 6;
char n1[size] = "shoe";
char n2[size] = "shine";

  bool before =
    lexicographical_compare(n1, n1 + size,
                             n2, n2 + size,
                             greater<char>());
  if(before)
    cout << n1 << " is after " << n2 << endl;
  else
    cout << n2 << " is after " << n1 << endl;
  return 0;
}
