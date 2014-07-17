#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define push(v)   *stackp++ = v
#define pop()      *--stackp
#define top()      *(stackp - 1)
#define isEmpty()   (stackp == buffer)
#define isAtom(c)   (c && (c) != '*' && (c) != '|' && (c) != '+' && (c) != '?' && (c) != ')')
#define put(x)      *outp++ = x
/*! 栈顶元素不低于输入,弹出栈顶 !*/

static char* re2post(const char *re){
    char buffer[1024];
    static char out[8000],*outp = out;
    char *stackp = buffer;
    const char *s = re;
    char ch,c;
    int atom = 0;
#define nextc() *(s + 1)
    for(c = *s;*s;){
        switch(c){
        case '(':
            push(c);
            break;
        case ')':
            while(!isEmpty()){
                ch = pop();
                if('(' == ch)
                    break;
                put(ch);
            }
            if(ch != '(' && isEmpty())
                goto fail;
            if(isAtom(nextc()))
                atom = 1;
            break;
        case '.':
            while(!isEmpty() && top() == '.')
                put(pop());
            push(c);
            break;
        case '|':
            while(!isEmpty() && top() != '(' && top() != '|')
                put(pop());
            push(c);
            break;
        case '*':
        case '+':
        case '?':
        default:
            put(c);
            if(isAtom(nextc()))
                atom = 1;
            break;
        }
        if(atom){
            atom = 0;
            c = '.';
        } else {
            c = *++s;
        }
    }
    while(!isEmpty()){
        if(top() == '(')
            goto fail;
        put(pop());
    }
    put('\0');
    return out;
fail:
    return NULL;
}

enum{
    Start   = 254,
    Match, /*! 该状态是接受状态 !*/ 
    E,     /*! 消耗空字符的状态 !*/
};

typedef struct state State;
typedef struct mach Mach;

struct state{
    int c;              /*! 输入这个字符到达这个状态 !*/
    union{
        State *out;     /*! 输入out->c字符到达的状态 !*/ 
        struct{ State *out1,*out2;};    /*! 输出e到达的状态 !*/
    };
};

struct mach{
    State *start;   /*! 开始状态 !*/
    State *match;   /*! 接受状态 !*/
};

static State *state(int c,State *out1,State *out2){
    State *s = malloc(sizeof(*s));    
    s->c = c;
    s->out1 = out1;
    s->out2 = out2;
    return s;
}

static Mach *mach(int c){
    Mach *m = malloc(sizeof(*m));
    m->match = state(c,NULL,NULL);
    m->start = state(E,m->match,NULL);
    return m;
}

static Mach *concat(Mach *m1,Mach *m2){
    m1->match->out = m2->start;
    m1->match = m2->match;
    free(m2);
    return m1;
}

static Mach *alliance(Mach *m1,Mach *m2){
    State *s1 = state(E,m1->start,m2->start);
    State *s2 = state(E,NULL,NULL);
    m1->match->out = s2;
    m2->match->out = s2;
    m1->start = s1;
    m1->match = s2;
    free(m2);
    return m1;
}

static Mach *zmore(Mach *m){
    State *s2 = state(E,NULL,NULL);
    State *s1 = state(E,m->start,s2);
    m->match->out1 = m->start;
    m->match->out2 = s2;
    m->start = s1;
    m->match = s2;
    return m;
}

static Mach *post2nfa(char *postfix){
    char *p;
    Mach *stack[1024],**stackp = stack,*m1,*m2,*m;
    if(!postfix)
        return NULL;
    for(p = postfix;*p;p++){
        switch(*p){
        default:
            push(mach(*p));
            break;
        case '.':
            m2 = pop();
            m1 = pop();
            push(concat(m1,m2));
            break;
        case '|':
            m2 = pop();
            m1 = pop();
            push(alliance(m1,m2));
            break;
        case '?':
            break;
        case '*':
            m = pop();
            push(zmore(m));
            break;
        }
    }
    return pop();
}

#define eq(x,a) ((x) && ((x)->c == a))

static State *step(State *st,char *s){
    State *st1;

    if(!st || !s)
        return st;

    //printf("[%c %c] -> ",st->c == E ? 'E' : st->c ,*s ?: '0');

    if(eq(st->out1,E)){
        st1 = step(st->out1,s);
        if(st1)
            return st1;
        else if(eq(st->out2,E)){
            st1 =  step(st->out2,s);
            if(st1)
                return st1;
        }
    } else if(*s && eq(st->out,*s))
        return step(st->out,s+1);
    else if(st->out)
        return NULL;

    return st;
}

static int match(Mach *m,char *s){
    State *st;
    st = step(m->start,s);
    //printf("Match %c\n",st ? (st->c == E ? 'E' : st->c ) : 'N');
    return st == m->match;
}

int main(int argc,char **argv){
    char *post;
    Mach *m;
    if(argc < 3){
        fprintf(stderr,"usage: reg regexp string...\n");
        return 1;
    }
    post = re2post(argv[1]);
    if(!post){
        fprintf(stdout,"Error> bad regexp %s\n",argv[1]);
        return 1;
    }
    printf("%s\n",post);
    m = post2nfa(post);

    if(!m){
        fprintf(stdout,"Error> error in post2nfa %s\n",post);
        return 1;
    }
    for(int i = 2;i < argc;i++){
        if(match(m,argv[i]))
            printf("\e[31m%s\e[0m\n",argv[i]);
    }
    return 0;
}
