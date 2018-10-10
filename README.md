# SPARC to C++

The **SPARC-2-CPP** project is developed to provide an interface between SPARC files and C++ applications. SPARC is a sorted answer set programming language with consistency restoring rules, and is mainly used as a way to represent problems that may be approached with a logic-based strategy in order to solve them, such as knowledge representation. In this way, the **SPARC-2-CPP** project is constituted by classes and methods that enable the extraction (from SPARC files) and manipulation of information for the construction of transition diagrams and POMDPs.
 
## Project limitations
Given that the project is in a permanent state of development, it is important to highlight its functionality limitations so that the user makes the most out of it. The limitiations of the last version are:

* **Partial checking**: When parsing a SPARC file to extract its sorts and rules it does not perform a full verification on the file's sintaxis, therefore, before using a SPARC file one should first run it with the [SPARC translator](https://github.com/iensen/sparc).

* **Accepted sort definitions**: Although SPARC enables a wide variety of ways to define a sort (for more detail on sorts and SPARC syntaxis see the [SPARC manual](https://github.com/iensen/sparc/wiki)), the current implementation only supports the following notations,

> **Explicit lists**: #car = {shelby, mustang, maverick}. #motorcycle = {indian, harley}. #color = {red, blue}.

> **Union of prev. defined sorts**: #vehicle = #car + #motorcycle.

> **Sorts with parameters**: #property = main_color(#vehicle,#color) + weight(#vehicle).

> **Combinations of the previous definitions**: #all_sorts = #vehicle + {dog, cat} + exchange(#car,#motorcycle).

* **Special notation**: Even though most of the functions work with any notation supported by SPARC, more specialized methods such as `sp::DomainSort::generateStates`, `sp::DomainSort::generateActions` and `sp::DomainSort::generateTransitions`, require a special notation:

> **Predicate val for fluent values**: The predicate val(#fluent,#boolean,#step) is defined to establish the truth value of fluents. For instance, val(main_color(indian,blue),true,1) says that the indian motorcycle's color was blue at time step 1.

> **Predicate occurs for events and actions**: The predicate occurs(#action,#step) represents when an action happened at a given time. For instance, occurs(start_engine(mustang),2) says that at time step 2 the mustang's engine was started.

* **1-level arguments at most**: The current implementation is limited to parse sorts that have at most arguments that in their own have no arguments, for instance see the following examples:

> **Supported sort definitions**: #sort_a = a1(#motorcycle) + {a2, a3}.

> **Unsupported sort definitions**: #sort_b = b2(#sort_a).

In these examples, `b2(#sort_a)` is not supported given that its argument is a sort that has at least 1 element that also has arguments. For more detail on the supported notation, see **examples/get-sorts.cpp**.

## Dependencies

* **SPARC**: In order for the some of the functions of **SPARC-2-CPP** to properly work it is necessary to have installed the [SPARC translator](https://github.com/iensen/sparc) and the ASP solver [Clingo](https://github.com/potassco/clingo/releases/), due to at the moment is the only solver that supports choice rules, which are required. Also, before running an example, the user should change the **PATH_TO_SPARC** constant in the **include/DomainSort.hpp** file, for the system to be able to find the SPARC translator.

