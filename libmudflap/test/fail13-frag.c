struct a {
  int x;
  int y;
  char z;
};

struct b {
  int x;
  int y;
};

struct b k;

(*((struct a*) &k)).z = 'q';

