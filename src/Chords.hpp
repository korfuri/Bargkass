#ifndef BARGKASS_CHORDS_HPP__
#define BARGKASS_CHORDS_HPP__

#include <string>
#include "jansson.h"

enum Third {
  third_maj, third_min, /*third_oct,*/ NUM_THIRDS
};
enum Fifth {
  fifth_prf, fifth_aug, fifth_dim, NUM_FIFTHS
};
enum Seventh {
  seventh_non, seventh_dim, seventh_min, seventh_maj, NUM_SEVENTHS
};
enum Inversion {
  inv_none, inv_first, inv_second, inv_third, NUM_INVERSIONS
};

template<typename T>
T& operator++(T& val) {
  return val = static_cast<T>( static_cast<int>(val) + 1 );
}


class Chord {
public:
  Chord(std::string root, float rootVOct);
  Chord(Chord const&);
  Chord();
  Chord& operator=(Chord const&);

  json_t* toJson() const;
  void fromJson(json_t *rootJ);
  
  void set_root(std::string root, float rootVOct);
  std::string root() const;
  std::string symbol() const;
  
  std::string describe() const;

  // Change intervals of the chord
  void shift_third();
  void shift_fifth();
  void shift_seventh();

  // Convenience functions to set common intervals
  void make_major();
  void make_minor();
  void make_seven();

  // openstring() returns what a note should be (in semitones from the
  // root) if this chord doesn't provide (e.g. fourth note in a
  // triad).
  int openstring() const;
  
  // Return intervals (in semitones) for each note in the chord
  // -1 represents that no note should play
  int intv_0() const;
  int intv_3() const;
  int intv_5() const;
  int intv_7() const;

  // Returns tensions in V/oct for the Nth note in the chord.
  // N starts at 0.
  float out(unsigned int n) const;
  
private:
  std::string root_;
  float voct_;
  Third third_ = third_maj;
  Fifth fifth_ = fifth_prf;
  Seventh seventh_ = seventh_non;
  Inversion inversion_ = inv_none;
};


#endif
