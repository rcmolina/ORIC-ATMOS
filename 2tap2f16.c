/* tap2f16 tool by S.Guyart thanks to F.Frances tap2wav tool 06/2016*/
/* TAP to FAST*1.6: should produce au 44kHz fastest possible tape signal with standard Oric ROMs */
/* v1.1 2016-08-06: uses a new waveform which should work on more Orics ( _--__ instead of _-___  )*/


#include <stdio.h>

FILE *in,*out;
int file_size;
int speed=44100;
int oldmachine=0;

struct {
	char sig[4];
	int riff_size;
	char typesig[4];
	char fmtsig[4];
	int fmtsize;
	short tag;
	short channels;
	int freq;
	int bytes_per_sec;
	short byte_per_sample;
	short bits_per_sample;
	char samplesig[4];
	int datalength;
} sample_riff= { "RIFF",0,"WAVE","fmt ",16,1,1,0,0,1,8,"data",0 };

void emit_level(int size, int niveau)
{
    int i;
    for (i=0;i<size;i++) fputc(niveau,out);
    file_size+=size;
}

void emit_silence()
{ /* silence pour les lecteurs mp3 ou autres qui "mangent" la fin du signal */
    static int current_level=0x80;
    int i, size;
    size=15000;
    current_level=256-current_level;
    for (i=0;i<size;i++) fputc(current_level,out);
    file_size+=size;
}

void emit_bit(int bit)
{
    switch (speed) {

      case 4800: /* 208,33us per pulse */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(1,192);
          emit_level(1+oldmachine,192);
        }
        break;
		
      case 5714: /* 175us per pulse */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(1,192);
          emit_level(1+oldmachine,192);
        }
        break;
		
      case 7692: /* 130us per pulse */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(1+oldmachine,192);
        }
        break;	   
	
      case 9600: /* 104,17us */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(2+oldmachine,192);
        }
        break;
		
      case 11538: /* 86,67us */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(3+oldmachine,192);
        }
        break;
		
      case 13461: /* 74,29us */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(4+oldmachine,192);
        }
        break;
		
      case 15384: /* 65us */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(5+oldmachine,192);
        }
        break;
		
      case 17094: /* 58.5us */
        if (bit) {
          emit_level(1,64);
          emit_level(1+oldmachine,192);
        } else {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(6+oldmachine,192);
        }
        break;	   	   	    	   
			 	 
			 
      case 44100: /* 22,67us */
        if (bit) {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(2+oldmachine,64);
        } else {
          emit_level(1,64);
          emit_level(2,192);
          emit_level(20+oldmachine,64);
        }
        break;	   	   
		
    }
}

void emit_byte(int val)
{
  int i,parity=1;
  emit_bit(0);
  for (i=0; i<8; i++,val>>=1) {
    parity+=val&1;
    emit_bit(val&1);
  }
  emit_bit(parity&1);
  emit_bit(1);
  emit_bit(1);
  emit_bit(1);  /* 3 bits stop au lieu de 3.5 */

}

void emit_gap()
{
  int i;
  /* un paquet de bits stop pour laisser le temps d'afficher la ligne de statut */
  for (i=0;i<100;i++) emit_bit(1); 
}

init(int argc, char *argv[])
{
  if (argc<3) return 1;
  if (argv[1][0]=='-') {
    if (feed_parameters(argv[1])) return 1;
  }
 if (argv[2][0]=='-') {
    if (feed_parameters(argv[2])) return 1;
  }

  sample_riff.freq=sample_riff.bytes_per_sec=speed;
  in=fopen(argv[argc-2],"rb");
  if (in==NULL) {
    printf("Cannot open %s file\n\n",argv[1]);
    return 1;
  }
  out=fopen(argv[argc-1],"wb");
  if (out==NULL) {
    printf("Cannot create %s file\n\n",argv[2]);
    return 1;
  }
  return 0;
}


int feed_parameters(char *param)
{
  switch (param[1]) {
    case 'o':
    case 'O':
      oldmachine = 1;
      break;
    case '4':
      speed=4800;
      break;
    case '5':
      speed=5714;
      break;	  	    	     	        
    case '7':
      speed=7692;
      break;	     
    case '9':
      speed=9600;
      break;
    case '1':
      if (atoi(param+1)==11)  speed=11538;
      else if (atoi(param+1)==13)  speed=13461;
      else if (atoi(param+1)==15)  speed=15384;     
      else if (atoi(param+1)==17)  speed=17094;  
      else {
        printf("Bad value: %s\n",param+1);
        return 1;
      }
      break;

    default:
      printf("Bad parameter: %s\n", param);
      return 1;
  }
  return 0;
}


main(int argc,char *argv[])
{
  int i,size;
  unsigned char header[9], valin;

  if (init(argc,argv)) {
    printf("Usage: %s [-4|-5|-7|-9|-11|-13|-15|-17] [ -o ] <.TAP file> <.WAV file>\n",argv[0]);
    printf("Options: -4  produces a 4800 Hz WAV file\n");
    printf("         -5  produces a 5714 Hz WAV file\n");	
    printf("         -7  produces a 7692 Hz WAV file\n");	    
    printf("         -9  produces a 9600 Hz WAV file\n");
    printf("         -11 produces a 11538 Hz WAV file\n");
    printf("         -13 produces a 13461 Hz WAV file\n");	
    printf("         -15 produces a 15384 Hz WAV file\n");		
    printf("         -17 produces a 17094 Hz WAV file (default is 44100 Hz)\n");
    printf("Produces a 44 kHz WAV file that should load on all standard Orics,\n");
    printf("but about 1.6 times faster than original standard CSAVE signal.\n\n");
    printf("Option : -o  'old machines' : adds a small delay to the shortest\n");
    printf("             periods. The resulting WAV is longer to load, but may\n");
    printf("             work if default fails to load on your Oric.\n");
    printf("(v1.1 - 08/2016)\n");
    exit(1);
  }

  fwrite(&sample_riff,1,sizeof(sample_riff),out);

  while (!feof(in)) {
    if (fgetc(in)==0x16) {      /* s'assurer qu'on a bien un debut de programme */
      do {
        valin=fgetc(in);        /* read synchro (0x24 included) */
        if (feof(in)) break;
      } while (valin!=0x24);
      if (feof(in)) break;
      for (i=0;i<256;i++) emit_byte(0x16); /* 256 octets de synchro */
      emit_byte(0x24);

      for (i=0;i<9;i++) emit_byte(header[i]=fgetc(in));  /* header */

      do {
        i=fgetc(in); emit_byte(i);   /* name */
      } while ((i!=0)&&(!feof(in)));

      emit_gap();

      size=(header[4]*256+header[5])-(header[6]*256+header[7])+1;
      for (i=0;i<size;i++) emit_byte(fgetc(in));
    }
  }
  fclose(in);

  emit_silence();

  sample_riff.datalength=file_size;
  sample_riff.riff_size=sample_riff.datalength+8+sample_riff.fmtsize+12;
  fseek(out,0,SEEK_SET); 
  fwrite(&sample_riff,1,sizeof(sample_riff),out);

  fclose(out);
  printf("Tap2f16 v1.1 (c) 08/2016 --- File converted.\n");
}

