"""Independent hand-written rule vectors and invariants for the Python API."""
import json
import random
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'examples/python'))
from engine import INITIAL, MAX_PLY, Engine, legal, parse_sfen, play, result, sfen
V = json.loads((ROOT / 'tests/vectors.json').read_text())


class Rules(unittest.TestCase):
    def test_golden_turns(self):
        for v in V['turns']:
            with self.subTest(v['name']):
                before = parse_sfen(v['before'])
                self.assertIn(v['move'], legal(before))
                after = play(before, v['move'])
                self.assertEqual(sfen(after), v['after'])
                self.assertEqual(sfen(before), v['before'])
                if 'result' in v:
                    self.assertEqual(result(after), v['result'])
                    self.assertEqual(legal(after), [])

    def test_illegal_and_atomic(self):
        for v in V['illegal']:
            with self.subTest(v['name']):
                text = v.get('before', INITIAL)
                state = parse_sfen(text)
                with self.assertRaisesRegex(ValueError, '^illegal$'):
                    play(state, v['move'])
                self.assertEqual(sfen(state), text)

    def test_invalid_sfen(self):
        for text in V['invalid_sfen']:
            with self.subTest(text):
                with self.assertRaisesRegex(ValueError, '^sfen$'):
                    parse_sfen(text)

    def test_syntax(self):
        for move in ['', '000', '0a1a2:1a1a2x', '0a1a2:1a1a2\n', '4a1a2:1a1a2', '0a0a2:1a1a2']:
            with self.assertRaisesRegex(ValueError, '^syntax$'):
                play(parse_sfen(INITIAL), move)

    def test_initial(self):
        s = parse_sfen(INITIAL)
        self.assertEqual(sfen(s), INITIAL)
        moves = legal(s)
        self.assertEqual(len(moves), 232)
        self.assertEqual(moves, sorted(set(moves)))
        self.assertEqual(moves[0], '0a1a2:1a1a2')
        self.assertEqual(result(s), 'ongoing')

    def test_blocked(self):
        # Black at a1, with b1/a2/b2 occupied, has no passive move on either homeboard.
        text = '4/4/WW2/BW2|4/4/WW2/BW2|3W/4/4/B3|3W/4/4/B3 b 0'
        s = parse_sfen(text)
        self.assertEqual(legal(s), [])
        self.assertEqual(result(s), 'w')

    def test_counter_limit(self):
        state = parse_sfen(INITIAL[:-1] + str(MAX_PLY-1))
        after = play(state, '0a1a2:1a1a2')
        self.assertEqual(after.ply, MAX_PLY)
        self.assertEqual(legal(after), [])
        self.assertEqual(result(after), 'ongoing')

    def test_repeated_positions_allowed(self):
        s = parse_sfen(INITIAL)
        for m in ['0a1a2:1a1a2', '2a4a3:3a4a3', '0a2a1:1a2a1', '2a3a4:3a3a4']*3:
            s = play(s,m)
        self.assertEqual(sfen(s), INITIAL[:-1]+'12')
        self.assertEqual(result(s), 'ongoing')

    def test_random_play_invariants(self):
        rng = random.Random(1701)
        for _ in range(5):
            s = parse_sfen(INITIAL)
            for _ in range(60):
                moves = legal(s)
                if not moves:
                    break
                t = play(s,rng.choice(moves))
                self.assertEqual(sfen(parse_sfen(sfen(t))), sfen(t))
                removed = 0
                for b in range(4):
                    for piece in 'BW':
                        delta = s.boards[b].count(piece)-t.boards[b].count(piece)
                        self.assertGreaterEqual(delta,0)
                        self.assertLessEqual(delta,1)
                        if piece == s.side.upper():
                            self.assertEqual(delta,0)
                        removed += delta
                self.assertLessEqual(removed,1)
                self.assertEqual(t.ply,s.ply+1)
                self.assertNotEqual(t.side,s.side)
                s = t

    def test_protocol_preservation(self):
        e = Engine()
        self.assertEqual(e.handle('state'),'error handshake')
        self.assertEqual(e.handle('hello 2'),'error version')
        self.assertEqual(e.handle('hello 1'),'ok usp 1')
        before = e.handle('state')
        for command in ['play bad','position bad','unknown','hello 2','play 2a1a2:3a1a2']:
            self.assertTrue(e.handle(command).startswith('error '))
            self.assertEqual(e.handle('state'),before)
        e.handle('go')
        self.assertEqual(e.handle('state'),before)


if __name__ == '__main__':
    unittest.main()
