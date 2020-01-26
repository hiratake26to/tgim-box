#include <iostream>
#include <vector>
#include <string>
#include <type_traits>

using namespace std;

int main() {
  vector<string> v;
  v.push_back("おはござー");
  v.push_back("ほんまぁ〜");
  v.push_back("ひまわりでーす🌻");

  for (const auto& i : v) {
    cout << i << endl;
    //i.push_back("🌈"); //error
  }

  //for (string&& i : v) { //error
  for (string& i : v) {
    // i is modifiable due to lvalue reference
    i += "+ほまぁ〜に？";
    cout << i << endl;
  }

  for (const auto& i : v) {
    cout << i << endl;
  }

}
