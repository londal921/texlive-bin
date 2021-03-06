/* Header for module status, generated by p2c 1.21alpha-07.Dec.93 */
#ifndef STATUS_H
#define STATUS_H


#ifdef STATUS_G
# define vextern
#else
# define vextern extern
#endif
/* Keep track of duration, octave, slur and beam status. */


extern void initStatus(void);
extern void saveStatus(short voice);

extern void resetDuration(short voice, Char dur);
extern Char duration(short voice);

extern short slurLevel(short voice);
extern short beamLevel(short voice);
extern boolean noBeamMelisma(short voice);
extern boolean noSlurMelisma(short voice, short history);
extern short afterSlur(short voice);
extern void setUnbeamed(short voice);
extern void setUnslurred(short voice);
extern void beginBeam(short voice, Char *note);
extern void endBeam(short voice);
extern void beginSlur(short voice, Char *note);
extern void endSlur(short voice, Char *note);
extern void activateBeamsAndSlurs(short voice);

extern void setOctave(short voice);
extern void resetOctave(short voice);
extern Char octave(short voice);
extern void newOctave(short voice, Char dir);
extern void initOctaves(Char *octaves);

extern void renewPitch(short voice, Char *note);
extern short chordPitch(short voice);
extern void renewChordPitch(short voice, Char *note);
extern void defineRange(short voice, Char *range);
extern void checkRange(short voice, Char *note);
extern void rememberDurations(void);
extern void restoreDurations(void);
extern void chordTie(short voice, Char *lab);


typedef short int5[5];


extern void getChordTies(short voice, short *pitches, Char *labels);


#undef vextern

#endif /*STATUS_H*/

/* End. */
