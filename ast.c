/*
 * helper functions for fb3-2
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "Untitled-1.h"

/* symbol table */
/* hash a symbol */

static unsigned
symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while (c = *sym++)
    hash = hash * 9 ^ c;

  return hash;
}
//void install(char *name, int v, char *type, int array_size);

void install(char *name, int v, char *type, int array_size)
{
  struct symbol *sp = lookup(name);
  sp->value = v;
  sp->type = strdup(type);
  sp->array_size = array_size;
}

void setvalue(char *a, int v)
{
  char *name = ((struct variable_declaration *)a)->name;
  struct symbol *sym = lookup(name);
  sym->value = v;
}

struct symbol *
lookup(char *sym)
{
  struct symbol *sp = &symtab[symhash(sym) % NHASH];
  int scount = NHASH; /* how many have we looked at */

  while (--scount >= 0)
  {
    if (sp->name && !strcmp(sp->name, sym))
    {
      return sp;
    }

    if (!sp->name)
    { /* new entry */
      sp->name = strdup(sym);
      sp->value = 0;
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if (++sp >= symtab + NHASH)
      sp = symtab; /* try the next entry */
  }
  printf("symbol table overflow\n");
  abort(); /* tried them all, table is full */
}

struct ast *
newast(int nodetype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    printf("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *
newnum(double d)
{
  struct numval *a = malloc(sizeof(struct numval));

  if (!a)
  {
    printf("out of space");
    exit(0);
  }
  a->nodetype = 'K';
  a->number = d;
  return (struct ast *)a;
}

struct ast *
newcmp(int cmptype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    printf("out of space");
    exit(0);
  }
  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *
newasgn(struct ast *s, struct ast *v)
{
  struct symasgn *a = malloc(sizeof(struct symasgn));

  if (!a)
  {
    printf("out of space");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

struct ast *
newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *ei, struct ast *el)
{
  struct flow *a = malloc(sizeof(struct flow));

  if (!a)
  {
    printf("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;
  a->ei = ei;
  return (struct ast *)a;
}

struct ast *
newforloop(int nodetype, struct ast *init, struct ast *cond, struct ast *incr, struct ast *body)
{
  struct forloop *a = malloc(sizeof(struct forloop));

  if (!a)
  {
    printf("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->init = init;
  a->cond = cond;
  a->incr = incr;
  a->body = body;
  return (struct ast *)a;
}

struct ast *
newvardec(int nodetype, char *type, char *name, int array_size, struct ast *init)
{
  struct variable_declaration *var = malloc(sizeof(struct variable_declaration));
  if (!var)
  {
    printf("Error allocating memory for variable declaration\n");
    exit(1);
  }

  var->nodetype = 'V';
  var->type = type;
  var->name = name;
  var->array_size = array_size;
  var->init = init;
  return (struct ast *)var;
}

struct ast *
newfndef(int nodetype, char *name, struct symbol *syms, struct ast *func)
{
  struct fndef *a = malloc(sizeof(struct fndef));
  if (!a)
  {
    printf("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->name = name;
  a->syms = syms;
  a->func = func;
  return (struct ast *)a;
}

struct ast *
newdecl(int nodetype, char *name, char *type, int array_size, struct ast *init)
{
  struct declaration_list *decl = malloc(sizeof(struct declaration_list));
  if (!decl)
  {
    printf("Error allocating memory for declaration\n");
    exit(1);
  }
  decl->nodetype = nodetype;
  decl->name = name;
  decl->type = type;
  decl->array_size = array_size;
  decl->init = init;
  decl->next = NULL;
  return (struct ast *)decl;
}
//void setvalue(char *name, int v);

/* free a tree of ASTs */
void treefree(struct ast *a)
{
  switch (a->nodetype)
  {

    /* two subtrees */
  case '+':
  case '-':
  case '*':
  case '/':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case 'L':
    treefree(a->r);

    /* one subtree */

    /* no subtree */
  case 'K':
    break;

  case '=':
    free(((struct symasgn *)a)->v);
    break;

    /* up to three subtrees */
  case 'I':
  case 'W':
    free(((struct flow *)a)->cond);
    if (((struct flow *)a)->tl)
      treefree(((struct flow *)a)->tl);
    if (((struct flow *)a)->el)
      treefree(((struct flow *)a)->el);
    break;

  case 'P':
    free(((struct forloop *)a)->init);
    free(((struct forloop *)a)->cond);
    free(((struct forloop *)a)->incr);
    treefree(((struct forloop *)a)->body);
    break;

  case 'D': // Function definition
    free(((struct fndef *)a)->name);
    if ((((struct fndef *)a)->func))
      treefree(((struct fndef *)a)->func);
    if (((struct fndef *)a)->syms)
      treefree(((struct fndef *)a)->syms);
    break;

  case 'V':
    free(((struct variable_declaration *)a)->name);
    free(((struct variable_declaration *)a)->type);
    if (((struct variable_declaration *)a)->init)
      treefree(((struct variable_declaration *)a)->init);
    break;

  case 'Y':
    free(((struct declaration_list *)a)->name);
    free(((struct declaration_list *)a)->type);
    if (((struct declaration_list *)a)->init)
      treefree(((struct declaration_list *)a)->init);
    break;

  case 'R':
    if (((struct return_statement *)a)->value)
      treefree(((struct return_statement *)a)->value);
    break;

  default:
    printf("internal error: free bad node %c\n", a->nodetype);
  }

  free(a); /* always free the node itself */
}

struct symlist *
newsymlist(struct symbol *sym, struct symlist *next)
{
  struct symlist *sl = malloc(sizeof(struct symlist));

  if (!sl)
  {
    printf("out of space");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  return sl;
}

struct ast *
newreturn(int nodetype, struct ast *value)
{
  struct return_statement *ret = malloc(sizeof(struct return_statement));
  if (!ret)
  {
    printf("Error allocating memory for return statement\n");
    exit(1);
  }

  ret->nodetype = nodetype;
  ret->value = value;
  return (struct ast *)ret;
}

/* free a list of symbols */
void symlistfree(struct symlist *sl)
{
  struct symlist *nsl;

  while (sl)
  {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}



double
eval(struct ast *a)
{
  double v;

  if (!a)
  {
    printf("internal error, null eval");
    return 0.0;
  }

  switch (a->nodetype)
  {
    /* constant */
  case 'K':
    v = ((struct numval *)a)->number;
    break;

    /* name reference */
 
    /* assignment */
  case '=':
    v = eval(((struct symasgn *)a)->v);
    if (((struct symref *)a)->s == NULL)
    {
      printf("Error: symbol not found in symbol table\n");
    }
    else
    {
      ((struct symref *)a)->s->value = v;
    }
    break;

    /* expressions */
  case '+':
    v = eval(a->l) + eval(a->r);
    break;
  case '-':
    v = eval(a->l) - eval(a->r);
    break;
  case '*':
    v = eval(a->l) * eval(a->r);
    break;
  case '/':
    v = eval(a->l) / eval(a->r);
    break;
  case 'M':
    v = -eval(a->l);
    break;

    /* comparisons */
  case '1':
    v = (eval(a->l) > eval(a->r)) ? 1 : 0;
    break;
  case '2':
    v = (eval(a->l) < eval(a->r)) ? 1 : 0;
    break;
  case '3':
    v = (eval(a->l) != eval(a->r)) ? 1 : 0;
    break;
  case '4':
    v = (eval(a->l) == eval(a->r)) ? 1 : 0;
    break;
  case '5':
    v = (eval(a->l) >= eval(a->r)) ? 1 : 0;
    break;
  case '6':
    v = (eval(a->l) <= eval(a->r)) ? 1 : 0;
    break;

  /* control flow */
  /* null expressions allowed in the grammar, so check for them */

  /* if/then/else */
  case 'I':
    if (eval(((struct flow *)a)->cond) != 0)
    { /*check the condition*/
      if (((struct flow *)a)->tl)
      { /*the true branch*/
        v = eval(((struct flow *)a)->tl);
      }
      else
      {
        v = 0.0;
      } /* a default value */
    }
    else
    {
      if (((struct flow *)a)->el)
      { /*the false branch*/
        v = eval(((struct flow *)a)->el);
      }
      else
        v = 0.0; /* a default value */
    }
    break;

  /* while/do */
  case 'W':
    v = 0.0; /* a default value */

    if (((struct flow *)a)->tl)
    {
      while (eval(((struct flow *)a)->cond) != 0) /*evaluate the condition*/
        v = eval(((struct flow *)a)->tl);         /*evaluate the target statements*/
    }
    break; /* value of last statement is value of while/do */

  case 'P':
    for (eval(((struct forloop *)a)->init); eval(((struct forloop *)a)->cond) != 0; eval(((struct forloop *)a)->incr))
    {
      v = eval(((struct forloop *)a)->body);
    }
    break;

  case 'D': // Function definition
    // Store function definition in symbol table
    ((struct fndef *)a)->syms->func = ((struct fndef *)a)->func;
    v = 0.0; // Return value is 0 for function definitions
    break;

   case 'V':
     v = 0.0; /* default value for variable declaration */
     /* add variable to symbol table */
     install(((struct variable_declaration *)a)->name, v, ((struct variable_declaration *)a)->type, ((struct variable_declaration *)a)->array_size);
     if (((struct variable_declaration *)a)->init)
     {
       /* assign initial value to variable */
       v = eval(((struct variable_declaration *)a)->init);
       setvalue(((struct variable_declaration *)a)->name, v);
     }
     break;

  case 'Y':
    v = 0.0; /* default value for variable declaration */
    /* add variable to symbol table */
   // install(((struct declaration_list *)a)->name, v, ((struct declaration_list *)a)->type, ((struct declaration_list *)a)->array_size);
    if (((struct declaration_list *)a)->init)
    {
      /* assign initial value to LIST */
      v = eval(((struct declaration_list *)a)->init);
     // setvalue(((struct declaration_list *)a)->name, v);
    }

    break;

  case 'R':
    /* Evaluate the return value */
    v = eval(((struct return_statement *)a)->value);

    break;

  /* list of statements */
  case 'L':
    eval(a->l);
    v = eval(a->r);
    break;

    case 'C':
    v = calluser((struct ufncall *)a);
    break;


  default:
    printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
}

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
  if (name->syms)
    symlistfree(name->syms);
  if (name->func)
    treefree(name->func);
  name->syms = syms;
  name->func = func;
}
static double calluser(struct ufncall *);
static double
calluser(struct ufncall *f)
{
  struct symbol *fn = f->s; /* function name */
  struct symlist *sl;       /* dummy arguments */
  struct ast *args = f->l;  /* actual arguments */
  double *oldval, *newval;  /* saved arg values */
  double v;
  int nargs;
  int i;

  if (!fn->func)
  {
    printf("call to undefined function %s", fn->name);
    return 0;
  }

  /* count the arguments */
  sl = fn->syms;
  for (nargs = 0; sl; sl = sl->next)
    nargs++;

  /* prepare to save them */
  oldval = (double *)malloc(nargs * sizeof(double));
  newval = (double *)malloc(nargs * sizeof(double));
  if (!oldval || !newval)
  {
    printf("Out of space in %s", fn->name);
    return 0.0;
  }

  /* evaluate the arguments */
  for (i = 0; i < nargs; i++)
  {
    if (!args)
    {
      printf("too few args in call to %s", fn->name);
      free(oldval);
      free(newval);
      return 0.0;
    }

    if (args->nodetype == 'L')
    { /* if this is a list node */
      newval[i] = eval(args->l);
      args = args->r;
    }
    else
    { /* if it's the end of the list */
      newval[i] = eval(args);
      args = NULL;
    }
  }

  /* save old values of dummies, assign new ones */
  sl = fn->syms;
  for (i = 0; i < nargs; i++)
  {
    struct symbol *s = sl->sym;

    oldval[i] = s->value;
    s->value = newval[i];
    sl = sl->next;
  }

  free(newval);
  /* evaluate the function */
  v = eval(fn->func);

  /* put the real values of the dummies back */
  sl = fn->syms;
  for (i = 0; i < nargs; i++)
  {
    struct symbol *s = sl->sym;

    s->value = oldval[i];
    sl = sl->next;
  }
  free(oldval);
  return v;
}

void print_symbol_table(struct symbol_table *table) {
    for (int i = 0; i < table->size; i++) {
        printf("%s: %d\n", table->symbols[i].name, table->symbols[i].value);
    }
}

void print_ast(struct ast *a) {
  /* Print the value of the current node */
  printf("%d", a->nodetype);
  /* Print the value of the left child */
  if (a->l) {
    printf("(");
    print_ast(a->l);
    printf(")");
  }
  /* Print the value of the right child */
  if (a->r) {
    printf("(");
    print_ast(a->r);
    printf(")");
  }
}

//  int main()
//  {
//    printf("> ");
//     // print_symbol_table(*table);
//    return yyparse();
//  }
