#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <random>
#include <memory>



int rand_between(int begin, int end);

typedef std::string symbol;
typedef std::vector<std::shared_ptr<symbol>> _molecule;

class molecule{
    public:
        bool operator==(const molecule&) const;
        molecule();
        ~molecule();
        _molecule vector;

};


template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}




namespace std {

  template <>
  struct hash<molecule>
  {
    std::size_t operator()(const molecule& k) const
    {
      using std::vector;
      using std::size_t;
      using std::hash;

      // Compute individual hash values for first,
      // second and third and combine them using XOR
      // and bit shifting:

      size_t size = k.vector.size();
      size_t seed = 0;
      for (size_t i = 0; i < size; i++){
                //Combine the hash of the current vector with the hashes of the previous ones
                hash_combine(seed, k.vector[i]);
      }
      return seed;

    //   return ((hash<_molecule*>()(&k.vector) << 1));
    }
  };

}


typedef std::shared_ptr<molecule> molecule_pointer;
typedef std::vector<molecule> moleculeVector;
//I cans makes this's ones faster.
typedef std::unordered_map<molecule,molecule_pointer> moleculeMap;
typedef std::unordered_multiset<molecule_pointer>   unorderedMultiset;





class moleculeMultiset {


    public:
        moleculeMultiset();
        ~moleculeMultiset();
        void inject(molecule_pointer mol, int mul);
        int expel(const molecule_pointer mol, int mult);
        const molecule_pointer expelrnd();
        const molecule_pointer rndMol();
        int mult(molecule_pointer& mol);
        int mult();
        unorderedMultiset multiset;

};