melody = \relative c' {
  \key c \major
  \time 4/4
  b4 c8 b8 a4 b4 f8 g8 a4 b4 e8 f8 g4 b2 b8 b8 b4 c8 b8 a4 a4 b8 a8 g4 g4 a8 g8 f4 e2.
}
\score {
  \new Staff \melody
  \layout { }
  \midi { }
}