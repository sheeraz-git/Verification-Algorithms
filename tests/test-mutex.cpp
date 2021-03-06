#include "verification-algorithms/k-induction/k-induction.hpp"
#include "verification-algorithms/ltl-bmc/ltl-bmc.hpp"
#include "verification-algorithms/ic3/ic3.hpp"
#include "verification-algorithms/pnet-coverability/pnet-coverability.hpp"

#include "catch2/catch.hpp"

using namespace verifier;

SCENARIO("mutual exclusion with propositional logic", "[mutex]") {
  std::vector<Symbol> symbols;
  for(int i = 1; i <= 5; ++i) {
    char ic = (i + '0');
    std::string var = "p";
    var += ic;
    Symbol pi(int_const, var);
    symbols.push_back(pi);
  }

  std::string I = "(p1 == 1 && p2 == 0 && p3 == 1 && p4 == 1 && p5 == 0)";

  std::string t[5];
  t[1] = "((p1 >= 1 && p3 >= 1) && (next_p1 == p1 - 1 && next_p2 == p2 + 1 && next_p3 == p3 - 1 && next_p4 == p4 && next_p5 == p5))";
  t[2] = "((p2 >= 1) && (next_p1 == p1 + 1 && next_p2 == p2 - 1 && next_p3 == p3 + 1  && next_p4 == p4 && next_p5 == p5))";
  t[3] = "((p4 >= 1 && p3 >= 1) && (next_p1 == p1 && next_p2 == p2 && next_p3 == p3 - 1 && next_p4 == p4 - 1 && next_p5 == p5 + 1))";
  t[4] = "((p5 >= 1) && (next_p1 == p1 && next_p2 == p2 && next_p3 == p3 + 1 && next_p4 == p4 + 1 && next_p5 == p5 - 1))";

  std::string T = "";
  for(int i = 1; i <= 4; ++i) {
    std::string clause = "(" + t[i];
    for(int j = 1; j <= 4; ++j) {
      if(j != i) {
       clause += " && !(" + t[j] + ")";
      }
    }
    clause += ")";
    if(i == 1) T = clause;
    else T += " || " + clause;
  }

  GIVEN("safety properties (for kInduction)") {
    kInduction k(symbols, I, T);
    std::string P;

    WHEN("property is mutual exclusion") {
      P = "(!(p2 > 0 && p5 > 0))";
      THEN("property holds") {
        REQUIRE(k.check(P) == true);
      }
    }

    WHEN("property is bad mutual exclusion") {
      P = "(p2 > 0 && p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(k.check(P) == false);
      }
    } 

    WHEN("property is always in critical section") {
      P = "(p2 > 0 || p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(k.check(P) == false);
      }
    }

    WHEN("property is tokenUsed") {
      P = "((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(k.check(P) == true);
      }
    }

    WHEN("property is tokenUnused") {
      P = "((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(k.check(P) == true);
      }
    }

    WHEN("property is badTokenUsed") {
      P = "((p3 == 0) -> (p2 == 1 && p5 == 1))";
      THEN("property does not hold") {
        REQUIRE(k.check(P) == false);
      }
    }

    WHEN("property has undeclared variable") {
      P = "(p1 == 0 || p10 == 0 || p6 == 0 || p51 == 0)";
      THEN("throws exception") {
        REQUIRE_THROWS_AS(k.check(P), std::invalid_argument);
      }
    }

    WHEN("property has bad assignment") {
      P = "(p1 == true)";
      THEN("throws bad_cast")  {
        REQUIRE_THROWS_AS(k.check(P), std::bad_cast);
      }
    }
  }

  GIVEN("ltl properties") {
    ltlBmc l(symbols, I, T);
    l.setBound(4);
    std::string P;

    WHEN("property is always mutual exclusion") {
      P = "G(!(p2 > 0 && p5 > 0))";
      THEN("property holds") {
        REQUIRE(l.check(P) == true);
      }
    }

    WHEN("property is always bad mutual exclusion") {
      P = "G(p2 > 0 && p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }
    } 

    WHEN("property is always in critical section") {
      P = "G(p2 > 0 || p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }
    }

    WHEN("property is always tokenUsed") {
      P = "G((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(l.check(P) == true);
      }
    }

    WHEN("property is always tokenUnused") {
      P = "G((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(l.check(P) == true);
      }
    }

    WHEN("property is always badTokenUsed") {
      P = "G((p3 == 0) -> (p2 == 1 && p5 == 1))";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }
    }

    WHEN("property is eventually critical section for thread 1") {
      P = "F(p2 > 0)";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }

      WHEN("property is eventually critical section for thread 2") {
        P = "F(p5 > 0)";
        THEN("property does not hold") {
          REQUIRE(l.check(P) == false);
        }
      }

      WHEN("property is eventually critical section for either") {
        P = "F(p2 > 0 || p5 > 0)";
        THEN("property holds") {
          REQUIRE(l.check(P) == true);
        }
      }

      WHEN("property has undeclared variable") {
        P = "F(p12)";
        THEN("throws invalid_argument") {
          REQUIRE_THROWS_AS(l.check(P), std::invalid_argument);
        }
      }

      WHEN("property has bad cast") {
        P = "F(p1 == true)";
        THEN("throws bad_cast") {
          REQUIRE_THROWS_AS(l.check(P), std::bad_cast);
        }
      }

      WHEN("property has bad cast 2") {
        P = "F(p1)";
        THEN("throws bad_cast") {
          REQUIRE_THROWS_AS(l.check(P), std::bad_cast);
        }
      }
    }
  } 
}

SCENARIO("mutual exclusion with pseudo-boolean formula", "[mutex]") {
  std::vector<Symbol> symbols;
  for(int i = 1; i <= 5; ++i) {
    char ic = (i + '0');
    std::string var = "p";
    var += ic;
    Symbol pi(int_const, var);
    symbols.push_back(pi);
  }

  std::string I = "(p1 == 1 && p2 == 0 && p3 == 1 && p4 == 1 && p5 == 0)";

  std::string t[5];
  t[1] = "((p1 >= 1 && p3 >= 1) && (next_p1 == p1 - 1 && next_p2 == p2 + 1 && next_p3 == p3 - 1 && next_p4 == p4 && next_p5 == p5))";
  t[2] = "((p2 >= 1) && (next_p1 == p1 + 1 && next_p2 == p2 - 1 && next_p3 == p3 + 1  && next_p4 == p4 && next_p5 == p5))";
  t[3] = "((p4 >= 1 && p3 >= 1) && (next_p1 == p1 && next_p2 == p2 && next_p3 == p3 - 1 && next_p4 == p4 - 1 && next_p5 == p5 + 1))";
  t[4] = "((p5 >= 1) && (next_p1 == p1 && next_p2 == p2 && next_p3 == p3 + 1 && next_p4 == p4 + 1 && next_p5 == p5 - 1))";

  std::string T = "";
  for(int i = 1; i <= 4; ++i) {
    if(i != 4) { T = T + t[i] + " + ";  }
    else { T = T + t[i] + " ==  1";  }
  }
  GIVEN("safety properties (for kInduction)") {
    kInduction k(symbols, I, T);
    std::string P;

    WHEN("property is mutual exclusion") {
      P = "(!(p2 > 0 && p5 > 0))";
      THEN("property holds") {
        REQUIRE(k.check(P) == true);
      }
    }

    WHEN("property is bad mutual exclusion") {
      P = "(p2 > 0 && p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(k.check(P) == false);
      }
    } 

    WHEN("property is always in critical section") {
      P = "(p2 > 0 || p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(k.check(P) == false);
      }
    }

    WHEN("property is tokenUsed") {
      P = "((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(k.check(P) == true);
      }
    }

    WHEN("property is tokenUnused") {
      P = "((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(k.check(P) == true);
      }
    }

    WHEN("property is badTokenUsed") {
      P = "((p3 == 0) -> (p2 == 1 && p5 == 1))";
      THEN("property does not hold") {
        REQUIRE(k.check(P) == false);
      }
    }
  }

  GIVEN("ltl properties") {
    ltlBmc l(symbols, I, T);
    l.setBound(4);
    std::string P;

    WHEN("property is always mutual exclusion") {
      P = "G(!(p2 > 0 && p5 > 0))";
      THEN("property holds") {
        REQUIRE(l.check(P) == true);
      }
    }

    WHEN("property is always bad mutual exclusion") {
      P = "G(p2 > 0 && p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }
    } 

    WHEN("property is always in critical section") {
      P = "G(p2 > 0 || p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }
    }

    WHEN("property is always tokenUsed") {
      P = "G((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(l.check(P) == true);
      }
    }

    WHEN("property is always tokenUnused") {
      P = "G((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(l.check(P) == true);
      }
    }

    WHEN("property is always badTokenUsed") {
      P = "G((p3 == 0) -> (p2 == 1 && p5 == 1))";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }
    }

    WHEN("property is eventually critical section for thread 1") {
      P = "F(p2 > 0)";
      THEN("property does not hold") {
        REQUIRE(l.check(P) == false);
      }

      WHEN("property is eventually critical section for thread 2") {
        P = "F(p5 > 0)";
        THEN("property does not hold") {
          REQUIRE(l.check(P) == false);
        }
      }

      WHEN("property is eventually critical section for either") {
        P = "F(p2 > 0 || p5 > 0)";
        THEN("property holds") {
          REQUIRE(l.check(P) == true);
        }
      }
    }
  } 
}

SCENARIO("mutual exclusion using smt", "[mutex-smt]") {
  std::vector<Symbol> symbols;
  for(int i = 1; i <= 5; ++i) {
    char ic = (i + '0');
    std::string var = "p";
    var += ic;
    Symbol pi(int_const, var);
    symbols.push_back(pi);
  }

  std::vector<std::string> I({"1", "0", "1", "1", "0"});
  std::vector<std::vector<std::string>> T({{"-1", "1", "0", "0"}, 
                                           {"1", "-1", "0", "0"}, 
                                           {"-1", "1", "-1", "1"}, 
                                           {"0", "0", "-1", "1"}, 
                                           {"0", "0", "1", "-1"}});
  GIVEN("safety properties") {
    PnetCoverability p(symbols, I, T);
    std::string P;

    WHEN("property is mutual exclusion") {
      P = "(!(p2 > 0 && p5 > 0))";
      THEN("property holds") {
        REQUIRE(p.check(P) == true);
      }
    }

    WHEN("property is bad mutual exclusion") {
      P = "(p2 > 0 && p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(p.check(P) == false);
      }
    } 

    WHEN("property is always in critical section") {
      P = "(p2 > 0 || p5 > 0)";
      THEN("property does not hold") {
        REQUIRE(p.check(P) == false);
      }
    }

    WHEN("property is tokenUsed") {
      P = "((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(p.check(P) == true);
      }
    }

    WHEN("property is tokenUnused") {
      P = "((p3 > 0) -> (p2 == 0 && p5 == 0))";
      THEN("property holds") {
        REQUIRE(p.check(P) == true);
      }
    }

    WHEN("property is badTokenUsed") {
      P = "((p3 == 0) -> (p2 == 1 && p5 == 1))";
      THEN("property does not hold") {
        REQUIRE(p.check(P) == false);
      }
    }
  }
}

