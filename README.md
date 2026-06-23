Hello! This is a wav mutation repository. It utilizes STFTs (both forward and inverse), a phase-vocoder, and the open-source libraries kiss_fft + AudioFile to allow for custom input files.

Rundown on how it works:
- Loads both wav files with separate AudioFile objects.
  
- Initializes fft plan + 2d arrays
  
- Runs forward STFTs
  
- Takes the magnitude of input file (phonems) and phase of output file (pitch)

- Utilizes phase-vocoder to preserve audio

- Takes the inverse STFT

- Instantiates a new AudioFile object and downloads to a pathname


STFT template is situated in the helper files.
