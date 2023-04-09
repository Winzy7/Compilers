good.cl

class A {
};

Class BB__ inherits A {
};

class Item {
   input: String;

   getString(): String {
      input
   };

   init(userInput: String): Item {
      {
      input <- userInput;
      self;
      }
   };

};


