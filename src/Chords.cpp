#include "Chords.hpp"


// Table of chord symbols indexed by [third][fifth][seventh]
static const std::string Symbols[NUM_THIRDS][NUM_FIFTHS][NUM_SEVENTHS] = {
  /*[third_maj] = */ { // 0-4
    /* [fifth_prf] = */ { // 0-4-7
      /*[seventh_non] = */ "", // major chord, 0-4-7
      /*      [seventh_dim] = */ "6", // sixth 0-4-7-9
      /*      [seventh_min] = */ "7", // seventh 0-4-7-10
      /*      [seventh_maj] = */ "maj7", // major seventh 0-4-7-11
    },
    /*    [fifth_aug] = */ { // 0-4-8
      /*      [seventh_non] = */ "aug", // augmented triad 0-4-8
      /*      [seventh_dim] = */ "aug6", // "Augmented sixth"? 0-4-8-9
      /*      [seventh_min] = */ "aug7", // Augmented seventh (dominant seventh sharp five) 0-4-8-10
      /*      [seventh_maj] = */ "augmaj7", // Augmented-major seventh (major seventh sharp five) 0-4-8-11
    },
    /*    [fifth_dim] = */ { // 0-4-6
      /*      [seventh_non] = */ "b5", // flat five, 0-4-6
      /*      [seventh_dim] = */ "6b5", // 0-4-6-9
      /*      [seventh_min] = */ "7b5", // 0-4-6-10
      /*      [seventh_maj] = */ "maj7b5", // 0-4-6-11
    },
  },
  /*  [third_min] = */ { // 0-3
    /*    [fifth_prf] = */ { // 0-3-7
      /*      [seventh_non] = */ "m", // minor chord 0-3-7
      /*      [seventh_dim] = */ "m6", // 0-3-7-9
      /*      [seventh_min] = */ "m7", // 0-3-7-10
      /*      [seventh_maj] = */ "m,maj7", // 0-3-7-11
    },
    /*    [fifth_aug] = */ { // 0-3-8
      /*      [seventh_non] = */ "m#5", // 0-3-8
      /*      [seventh_dim] = */ "m6#5", // 0-3-8-9
      /*      [seventh_min] = */ "m7#5", // 0-3-8-10
      /*      [seventh_maj] = */ "m,maj7#5", // 0-3-8-11
    },
    /*    [fifth_dim] = */ { // 0-3-6
      /*      [seventh_non] = */ "dim", // diminished triad 0-3-6
      /*      [seventh_dim] = */ "dim7", // 0-3-6-9
      /*      [seventh_min] = */ "m7b5", // 0-3-6-10
      /*      [seventh_maj] = */ "dim,maj7", // 0-3-6-11
    },
  },
};

Chord::Chord(std::string root, float rootVOct) : root_(root), voct_(rootVOct) {}

Chord::Chord() : Chord("C", 0.f) {}

Chord::Chord(Chord const& rhs) :
  root_(rhs.root_),
  voct_(rhs.voct_),
  third_(rhs.third_),
  fifth_(rhs.fifth_),
  seventh_(rhs.seventh_),
  inversion_(rhs.inversion_) {}

Chord& Chord::operator=(Chord const& rhs) {
  root_ = rhs.root_;
  voct_ = rhs.voct_;
  third_ = rhs.third_;
  fifth_ = rhs.fifth_;
  seventh_ = rhs.seventh_;
  inversion_ = rhs.inversion_;
  return *this;
}

json_t* Chord::toJson() const {
  json_t *rootJ = json_object();
  json_object_set_new(rootJ, "root_name", json_string(root_.c_str()));
  json_object_set_new(rootJ, "root_voct", json_real(voct_));
  json_object_set_new(rootJ, "third", json_integer((int)third_));
  json_object_set_new(rootJ, "fifth", json_integer((int)fifth_));
  json_object_set_new(rootJ, "seventh", json_integer((int)seventh_));
  json_object_set_new(rootJ, "inversion", json_integer((int)inversion_));
  return rootJ;
}


void Chord::fromJson(json_t *rootJ) {
  json_t *val;
  if ((val = json_object_get(rootJ, "root_name")))
    root_ = json_string_value(val);
  if ((val = json_object_get(rootJ, "root_voct")))
    voct_ = json_real_value(val);
  if ((val = json_object_get(rootJ, "third")))
    third_ = (Third)json_integer_value(val);
  if ((val = json_object_get(rootJ, "fifth")))
    fifth_ = (Fifth)json_integer_value(val);
  if ((val = json_object_get(rootJ, "seventh")))
    seventh_ = (Seventh)json_integer_value(val);
  if ((val = json_object_get(rootJ, "inversion")))
    inversion_ = (Inversion)json_integer_value(val);
}

void Chord::set_root(std::string root, float rootVOct) {
  root_ = root;
  voct_ = rootVOct;
}

std::string Chord::root() const {
  return root_;
}

std::string Chord::symbol() const {
  return Symbols[third_][fifth_][seventh_];
}

std::string Chord::describe() const {
  return root() + symbol();
};

void Chord::shift_third() {
  if (++third_ >= NUM_THIRDS) third_ = third_maj;
}

void Chord::shift_fifth() {
  if (++fifth_ >= NUM_FIFTHS) fifth_ = fifth_prf;
}

void Chord::shift_seventh() {
  if (++seventh_ >= NUM_SEVENTHS) seventh_ = seventh_non;
}

void Chord::make_major() {
  third_ = third_maj;
  fifth_ = fifth_prf;
  seventh_ = seventh_non;
}

void Chord::make_minor() {
  third_ = third_min;
  fifth_ = fifth_prf;
  seventh_ = seventh_non;
}

void Chord::make_seven() {
  if (seventh_ == seventh_non) {
    seventh_ = seventh_min;
  } else {
    seventh_ = seventh_non;
  }
}

int Chord::openstring() const {
  return 0; // TODO: consider playing the octave, or letting the user pick
}

int Chord::intv_0() const {
  return 0;
}

int Chord::intv_3() const {
  switch (third_) {
  case third_maj: return 4;
  case third_min: return 3;
  default: return openstring();
  }
}

int Chord::intv_5() const {
  switch (fifth_) {
  case fifth_prf: return 7;
  case fifth_aug: return 8;
  case fifth_dim: return 6;
  default: return openstring();
  }
}

int Chord::intv_7() const {
  switch (seventh_) {
  case seventh_non: return openstring();
  case seventh_dim: return 9;
  case seventh_min: return 10;
  case seventh_maj: return 11;
  default: return openstring();
  }
}


float Chord::out(unsigned int n) const {
  if (n >= 4) return 0.f;
  // TODO: inversions
  if (n == 0) return voct_ + (float)intv_0() / 12.f;
  if (n == 1) return voct_ + (float)intv_3() / 12.f;
  if (n == 2) return voct_ + (float)intv_5() / 12.f;
  if (n == 3) return voct_ + (float)intv_7() / 12.f;
}
