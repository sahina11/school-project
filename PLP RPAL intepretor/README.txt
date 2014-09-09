The purpose of this project is to create an interpreter of RPAL, which is a functional language.

The example source code for RPAL: 
Print (
        PP ('program',
	    'p01.t',
	     nil aug 'dclns',
             ('block',
		   ('output', 'read')
             ),
	    'p01.t'
	   ) (nil aug 3)
      )
// output: (3)

The processing flow:

source code --> scanner --> create AST --> Standardize --> Create control signal --> CSE machine --> output result

~scanner: The input is raw source code, just a text file of RPAL program.  The output is tokens. What we do here is to convert a sequence of characters into s sequence of tokens, meaningful character strings, by taking lexical grammar into account.
~AST:  Abstract syntax tree.  Here we convert a sequence of tokens into a tree structure to represent the source code.
~Standardize: We further process the AST into a standardized form. It is still a tree.
~Control signal: By traversing Standardized tree structure, we could create control signals. These control signal will be feed into next stage, CSE machine, to get the final result.
~CSE: control, stack, environment machine. This is an mechanism to systematically evaluate RPAL programs. Remember, the program here has been processed into control structures. The output is the result of program.
