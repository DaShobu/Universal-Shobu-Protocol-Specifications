// USP 1.0 reference engine, Node.js 18+, no dependencies.
import process from 'node:process';
import { pathToFileURL } from 'node:url';
export const INITIAL = Array(4).fill('WWWW/4/4/BBBB').join('|') + ' b 0';
const MAX = 2147483647;
const pattern = /^[0-3][a-d][1-4][a-d][1-4]:[0-3][a-d][1-4][a-d][1-4]$/;
const fail = code => { throw new Error(code); };
export function parseSFEN(text) {
  if (/[^\x20-\x7e]/.test(text) || !/^[BW1-4/|]+ [bw] (0|[1-9][0-9]{0,9})$/.test(text)) fail('sfen');
  const [layout, side, counter] = text.split(' ');
  if (+counter > MAX) fail('sfen');
  const parts = layout.split('|');
  if (parts.length !== 4) fail('sfen');
  const boards = parts.map(part => {
    const ranks = part.split('/');
    if (ranks.length !== 4) fail('sfen');
    const board = ranks.reverse().flatMap(rank => {
      const row = []; let digit = false;
      for (const c of rank) {
        if ('1234'.includes(c)) {
          if (digit) fail('sfen');
          row.push(...Array(+c).fill('.')); digit = true;
        } else { row.push(c); digit = false; }
      }
      if (row.length !== 4) fail('sfen');
      return row;
    });
    for (const c of ['B','W']) if (board.filter(v => v === c).length > 4) fail('sfen');
    return board;
  });
  if (boards.some(b => !b.includes('B')) && boards.some(b => !b.includes('W'))) fail('sfen');
  return { boards, side, ply: +counter };
}
export function sfen(s) {
  return s.boards.map(b => [3,2,1,0].map(y => {
    let row = '', empty = 0;
    for (const c of b.slice(y*4,y*4+4)) {
      if (c === '.') empty++;
      else { if (empty) row += empty; empty = 0; row += c; }
    }
    return row + (empty || '');
  }).join('/')).join('|') + ` ${s.side} ${s.ply}`;
}
function decode(m) {
  if (m.length !== 11 || !pattern.test(m)) fail('syntax');
  return [m.slice(0,5),m.slice(6)].map(t => [+t[0],t.charCodeAt(1)-97,+t[2]-1,t.charCodeAt(3)-97,+t[4]-1]);
}
function emptyWinner(s) {
  if (s.boards.some(b => !b.includes('B'))) return 'w';
  if (s.boards.some(b => !b.includes('W'))) return 'b';
  return null;
}
export function applyRaw(s,m) {
  const [[pb,x,y,tx,ty],[ab,ax,ay,atx,aty]] = decode(m);
  const dx=tx-x, dy=ty-y, n=Math.max(Math.abs(dx),Math.abs(dy));
  if (Math.floor(pb/2) !== (s.side==='b'?0:1) || pb%2===ab%2 || ![1,2].includes(n)
      || ![0,n].includes(Math.abs(dx)) || ![0,n].includes(Math.abs(dy)) || atx-ax!==dx || aty-ay!==dy) fail('illegal');
  const ux=dx/n,uy=dy/n,own=s.side.toUpperCase();
  const out={boards:s.boards.map(b=>b.slice()),side:s.side==='b'?'w':'b',ply:s.ply+1};
  for (const [bid,sx0,sy0,aggressive] of [[pb,x,y,false],[ab,ax,ay,true]]) {
    let sx=sx0,sy=sy0,pushed=null; const b=out.boards[bid];
    if (b[sy*4+sx]!==own) fail('illegal');
    for (let i=0;i<n;i++) {
      const nx=sx+ux,ny=sy+uy;
      if (nx<0||nx>3||ny<0||ny>3) fail('illegal');
      const target=ny*4+nx;
      if (b[target]!=='.') {
        if (!aggressive||b[target]===own||(pushed!==null&&pushed!==target)) fail('illegal');
        const px=nx+ux,py=ny+uy,next=px>=0&&px<4&&py>=0&&py<4?py*4+px:-1;
        if (next>=0) { if (b[next]!=='.') fail('illegal'); b[next]=b[target]; }
        pushed=next;
      }
      b[sy*4+sx]='.'; b[target]=own; sx=nx;sy=ny;
    }
  }
  return out;
}
export function legal(s,enforceLimit=true) {
  if (emptyWinner(s)||(enforceLimit&&s.ply===MAX)) return [];
  const moves=[];
  for(let pb=0;pb<4;pb++) for(let src=0;src<16;src++) {
    if(Math.floor(pb/2)!==(s.side==='b'?0:1)||s.boards[pb][src]!==s.side.toUpperCase()) continue;
    const x=src%4,y=Math.floor(src/4);
    for(let ux=-1;ux<=1;ux++) for(let uy=-1;uy<=1;uy++) for(let n=1;n<=2;n++) {
      if(!ux&&!uy) continue;
      const tx=x+ux*n,ty=y+uy*n;
      if(tx<0||tx>3||ty<0||ty>3) continue;
      for(let ab=0;ab<4;ab++) for(let src2=0;src2<16;src2++) {
        if(pb%2===ab%2||s.boards[ab][src2]!==s.side.toUpperCase()) continue;
        const ax=src2%4,ay=Math.floor(src2/4),atx=ax+ux*n,aty=ay+uy*n;
        if(atx<0||atx>3||aty<0||aty>3) continue;
        const sq=(x,y)=>String.fromCharCode(97+x)+(y+1);
        const m=`${pb}${sq(x,y)}${sq(tx,ty)}:${ab}${sq(ax,ay)}${sq(atx,aty)}`;
        try {applyRaw(s,m);moves.push(m);} catch(e) {if(e.message!=='illegal') throw e;}
      }
    }
  }
  return moves.sort();
}
export function result(s) {return emptyWinner(s)||(legal(s,false).length?'ongoing':s.side==='b'?'w':'b');}
export function play(s,m) {
  decode(m);
  if(s.ply===MAX||emptyWinner(s)) fail('illegal');
  return applyRaw(s,m);
}
export class Engine {
  constructor(){this.state=parseSFEN(INITIAL);this.negotiated=false;}
  handle(line) {
    if(!line||line.length>4096||line.startsWith(' ')||line.endsWith(' ')||line.includes('  ')||/[^\x20-\x7e]/.test(line)) return 'error syntax';
    if(line==='quit') return 'ok bye';
    if(line==='hello'||line.startsWith('hello ')) {
      if(line!=='hello 1') return 'error version';
      this.negotiated=true;return 'ok usp 1';
    }
    if(!this.negotiated) return 'error handshake';
    try {
      if(line==='ready') return 'ok ready';
      if(line==='new'||line==='position startpos') {this.state=parseSFEN(INITIAL);return line==='new'?'ok new':'ok position';}
      if(line==='position'||line.startsWith('position ')) {this.state=parseSFEN(line.slice(9));return 'ok position';}
      if(line==='play'||line.startsWith('play ')) {this.state=play(this.state,line.slice(5));return 'ok play';}
      if(line==='state') return 'state '+sfen(this.state);
      if(line==='result') return 'result '+result(this.state);
      if(line==='legal'||line==='go') {const moves=legal(this.state);return line==='legal'?'legal'+moves.map(m=>' '+m).join(''):'bestmove '+(moves[0]||'none');}
      return 'error syntax';
    } catch(e) {if(!['sfen','syntax','illegal'].includes(e.message)) throw e;return 'error '+e.message;}
  }
}
function main() {
  const engine=new Engine(); let bytes=[],oversized=false,done=false;
  // Stream bytes directly: readline would allocate an unbounded malicious line.
  process.stdin.on('data',chunk=>{
    if(done) return;
    for(const byte of chunk) {
      if(byte===10) {
        if(bytes.at(-1)===13) bytes.pop();
        const reply=oversized?'error syntax':engine.handle(Buffer.from(bytes).toString('latin1'));
        process.stdout.write(reply+'\n');bytes=[];oversized=false;
        if(reply==='ok bye'){done=true;process.stdin.destroy();break;}
      } else if(bytes.length<4098) bytes.push(byte); else oversized=true;
    }
  });
  // EOF deliberately discards a partial final request.
}
if(process.argv[1] && import.meta.url===pathToFileURL(process.argv[1]).href) main();
