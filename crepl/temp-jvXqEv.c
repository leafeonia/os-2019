int f(){return 1;}
int __expr_wrap_1() {return f();}
int __expr_wrap_2() {return 1+2;}
int g(){return 2;}
int __expr_wrap_3() {return g();}
int __expr_wrap_4() {return 3+5;}
int __expr_wrap_5() {return 3*17;}
//t fact(int n){if (n == 0) return 1; else return fact(n-1)*n}
int fact(int n){if (n == 0) return 1;else return fact(n-1)*n;}
int __expr_wrap_6() {return fact(3);}
int __expr_wrap_7() {return fact(10);}
int __expr_wrap_8() {return fact(0);}
int __expr_wrap_9() {return fact(-1);}
