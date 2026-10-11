#include "shobu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

const char *const shobu_initial = "WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB b 0";
static int inside(int x,int y) { return x>=0 && x<4 && y>=0 && y<4; }
static char own(const Shobu *s) { return s->side=='b'?'B':'W'; }
static char empty_winner(const Shobu *s) {
    int eb=0,ew=0;
    for(int b=0;b<4;b++) {
        int nb=0,nw=0;
        for(int i=0;i<16;i++) { nb+=s->board[b][i]=='B'; nw+=s->board[b][i]=='W'; }
        eb|=!nb; ew|=!nw;
    }
    return eb?'w':ew?'b':'-';
}
int shobu_parse(const char *text,Shobu *out) {
    Shobu s; const char *p=text; int eb=0,ew=0;
    memset(&s,0,sizeof s);
    for(int b=0;b<4;b++) {
        int nb=0,nw=0;
        for(int y=3;y>=0;y--) {
            int x=0,previous_digit=0;
            while(*p && *p!='/' && *p!='|' && *p!=' ') {
                char c=*p++;
                if(c>='1'&&c<='4') {
                    if(previous_digit||x+c-'0'>4) return 0;
                    for(int k=0;k<c-'0';k++) s.board[b][y*4+x++]='.';
                    previous_digit=1;
                } else if(c=='B'||c=='W') {
                    if(x>=4) return 0;
                    s.board[b][y*4+x++]=c; nb+=c=='B';nw+=c=='W';previous_digit=0;
                } else return 0;
            }
            if(x!=4||*p!=(y?'/':b<3?'|':' ')) return 0;
            p++;
        }
        if(nb>4||nw>4) return 0;
        eb|=!nb;ew|=!nw;
    }
    if(eb&&ew) return 0;
    if(*p!='b'&&*p!='w') return 0;
    s.side=*p++;
    if(*p++!=' ') return 0;
    if(*p<'0'||*p>'9'||(*p=='0'&&p[1])) return 0;
    uint32_t n=0;
    while(*p) {
        if(*p<'0'||*p>'9') return 0;
        unsigned d=(unsigned)(*p++-'0');
        if(n>((uint32_t)USP_MAX_PLY-d)/10) return 0;
        n=n*10+d;
    }
    s.ply=(int32_t)n; *out=s;return 1;
}
void shobu_sfen(const Shobu *s,char out[USP_SFEN_SIZE]) {
    char *p=out;
    for(int b=0;b<4;b++) {
        if(b) *p++='|';
        for(int y=3;y>=0;y--) {
            if(y<3) *p++='/';
            int empty=0;
            for(int x=0;x<4;x++) {
                char c=s->board[b][y*4+x];
                if(c=='.') empty++;
                else { if(empty) *p++=(char)('0'+empty);empty=0;*p++=c; }
            }
            if(empty) *p++=(char)('0'+empty);
        }
    }
    snprintf(p,USP_SFEN_SIZE-(size_t)(p-out)," %c %ld",s->side,(long)s->ply);
}
static int decode(const char *m,int v[10]) {
    if(strlen(m)!=11||m[5]!=':') return 0;
    for(int leg=0;leg<2;leg++) {
        const char *p=m+leg*6; int *a=v+leg*5;
        if(p[0]<'0'||p[0]>'3'||p[1]<'a'||p[1]>'d'||p[2]<'1'||p[2]>'4'||p[3]<'a'||p[3]>'d'||p[4]<'1'||p[4]>'4') return 0;
        a[0]=p[0]-'0';a[1]=p[1]-'a';a[2]=p[2]-'1';a[3]=p[3]-'a';a[4]=p[4]-'1';
    }
    return 1;
}
static int apply_raw(const Shobu *s,const char *m,Shobu *out) {
    int v[10]; if(!decode(m,v)) return 1;
    int dx=v[3]-v[1],dy=v[4]-v[2],n=abs(dx)>abs(dy)?abs(dx):abs(dy);
    if(v[0]/2!=(s->side=='b'?0:1)||v[0]%2==v[5]%2||n<1||n>2||
       (abs(dx)!=0&&abs(dx)!=n)||(abs(dy)!=0&&abs(dy)!=n)||v[8]-v[6]!=dx||v[9]-v[7]!=dy) return 2;
    int ux=dx/n,uy=dy/n;Shobu t=*s;
    for(int leg=0;leg<2;leg++) {
        int *a=v+leg*5,x=a[1],y=a[2],pushed=-2;char *b=t.board[a[0]];
        if(b[y*4+x]!=own(s)) return 2;
        for(int step=0;step<n;step++) {
            int nx=x+ux,ny=y+uy;
            if(!inside(nx,ny)) return 2;
            int target=ny*4+nx;
            if(b[target]!='.') {
                if(!leg||b[target]==own(s)||(pushed!=-2&&pushed!=target)) return 2;
                int px=nx+ux,py=ny+uy,next=inside(px,py)?py*4+px:-1;
                if(next>=0) {if(b[next]!='.') return 2;b[next]=b[target];}
                pushed=next;
            }
            b[y*4+x]='.';b[target]=own(s);x=nx;y=ny;
        }
    }
    t.side=s->side=='b'?'w':'b';
    /* Internal generation at the representation limit needs no increment. */
    if(t.ply<USP_MAX_PLY) t.ply++;
    *out=t;return 0;
}
int shobu_play(const Shobu *s,const char *move,Shobu *out) {
    int v[10];if(!decode(move,v)) return 1;
    if(s->ply==USP_MAX_PLY||empty_winner(s)!='-') return 2;
    return apply_raw(s,move,out);
}
static int compare_moves(const void *a,const void *b) {
    return strcmp(((const ShobuMove*)a)->text,((const ShobuMove*)b)->text);
}
int shobu_legal(const Shobu *s,ShobuMove out[USP_MAX_MOVES],int enforce_limit) {
    if(empty_winner(s)!='-'||(enforce_limit&&s->ply==USP_MAX_PLY)) return 0;
    int count=0;
    for(int pb=0;pb<4;pb++) for(int src=0;src<16;src++) {
        if(pb/2!=(s->side=='b'?0:1)||s->board[pb][src]!=own(s)) continue;
        int x=src%4,y=src/4;
        for(int ux=-1;ux<=1;ux++) for(int uy=-1;uy<=1;uy++) for(int n=1;n<=2;n++) {
            int tx=x+ux*n,ty=y+uy*n;if((!ux&&!uy)||!inside(tx,ty)) continue;
            for(int ab=0;ab<4;ab++) for(int src2=0;src2<16;src2++) {
                if(pb%2==ab%2||s->board[ab][src2]!=own(s)) continue;
                int ax=src2%4,ay=src2/4,atx=ax+ux*n,aty=ay+uy*n;
                if(!inside(atx,aty)) continue;
                char m[12];Shobu next;
                snprintf(m,sizeof m,"%c%c%c%c%c:%c%c%c%c%c",'0'+pb,'a'+x,'1'+y,'a'+tx,'1'+ty,'0'+ab,'a'+ax,'1'+ay,'a'+atx,'1'+aty);
                if(!apply_raw(s,m,&next)) {strcpy(out[count++].text,m);}
            }
        }
    }
    qsort(out,(size_t)count,sizeof *out,compare_moves);return count;
}
char shobu_result(const Shobu *s) {
    char w=empty_winner(s);if(w!='-') return w;
    ShobuMove moves[USP_MAX_MOVES];
    return shobu_legal(s,moves,0)?'-':s->side=='b'?'w':'b';
}
static int valid_line(const char *line) {
    size_t n=strlen(line);
    if(!n||n>4096||line[0]==' '||line[n-1]==' '||strstr(line,"  ")) return 0;
    for(size_t i=0;i<n;i++) if((unsigned char)line[i]<32||(unsigned char)line[i]>126) return 0;
    return 1;
}
int shobu_run(void) {
#ifdef _WIN32
    /* Avoid CRT CRLF/Ctrl-Z translation; framing is defined over raw bytes. */
    if(_setmode(_fileno(stdin),_O_BINARY)==-1||_setmode(_fileno(stdout),_O_BINARY)==-1) return 1;
#endif
    Shobu state;int negotiated=0;shobu_parse(shobu_initial,&state);
    for(;;) {
        char line[4099];size_t length=0;int overflow=0,c;
        while((c=getchar())!=EOF&&c!='\n') {if(length<4098) line[length++]=(char)c;else overflow=1;}
        if(c==EOF) {if(ferror(stdin)) return 1;break;}
        if(length&&line[length-1]=='\r') length--;
        line[length]='\0';
        /* NUL is invalid even though C strings terminate at it. */
        if(overflow||memchr(line,0,length)||!valid_line(line)) puts("error syntax");
        else if(!strcmp(line,"quit")) {puts("ok bye");if(fflush(stdout)==EOF||ferror(stdout)) return 1;break;}
        else if(!strcmp(line,"hello")||!strncmp(line,"hello ",6)) {
            if(strcmp(line,"hello 1")) puts("error version");
            else {negotiated=1;puts("ok usp 1");}
        } else if(!negotiated) puts("error handshake");
        else if(!strcmp(line,"ready")) puts("ok ready");
        else if(!strcmp(line,"new")||!strcmp(line,"position startpos")) {
            shobu_parse(shobu_initial,&state);puts(!strcmp(line,"new")?"ok new":"ok position");
        } else if(!strcmp(line,"position")||!strncmp(line,"position ",9)) {
            Shobu next;
            if(shobu_parse(strlen(line)>8?line+9:"",&next)) {state=next;puts("ok position");} else puts("error sfen");
        } else if(!strcmp(line,"play")||!strncmp(line,"play ",5)) {
            Shobu next;int e=shobu_play(&state,strlen(line)>4?line+5:"",&next);
            if(!e) {state=next;puts("ok play");}else puts(e==1?"error syntax":"error illegal");
        } else if(!strcmp(line,"state")) {char text[USP_SFEN_SIZE];shobu_sfen(&state,text);printf("state %s\n",text);}
        else if(!strcmp(line,"result")) {char r=shobu_result(&state);if(r=='-') puts("result ongoing");else printf("result %c\n",r);}
        else if(!strcmp(line,"legal")||!strcmp(line,"go")) {
            ShobuMove moves[USP_MAX_MOVES];int n=shobu_legal(&state,moves,1);
            if(!strcmp(line,"go")) printf("bestmove %s\n",n?moves[0].text:"none");
            else {fputs("legal",stdout);for(int i=0;i<n;i++) printf(" %s",moves[i].text);putchar('\n');}
        } else puts("error syntax");
        if(fflush(stdout)==EOF||ferror(stdout)) return 1;
    }
    return 0;
}
