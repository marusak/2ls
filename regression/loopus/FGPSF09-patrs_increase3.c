int nondet();

void increase3(int a, int b, int c) {
    while (1) {
      if (a >= b + 1 && a >= c + 1) {
        b = b + 1;
      }
      else if (a >= b + 1 && a >= c + 1) {
        c = c + 1;
      }
      else
        return;
    }
}
