#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef char bool;
#define true ((bool)1)
#define false ((bool)0)

#define LINES_PER_MALLOC   ((size_t)256)
#define BYTES_PER_MALLOC   ((size_t)65536)
#define MAX_REPLSTRING_LEN 256

/* functions <<< */
void printHelp(void)
{
   printf("USAGE: loco [-vhpdis] [-r <char>] [-R <string>] [file]\n");
   printf("\n");
   printf("OPTIONS:\n");
   printf("  -p          print longest common string\n");
   printf("  -d          print without longest common string\n");
   printf("  -i          ignore first line completely, second line becomes first line\n");
   printf("  -s          skip, do not change first non-ignored line\n");
   printf("  -r <char>   replace each character of longest common string with <char>\n");
   printf("  -R <string> replace longest common string with <string>\n");
   printf("  -v          print version info\n");
   printf("  -h          print help text\n");
   printf("\n");
}

void printUsage(void)
{
   fprintf(stderr, "USAGE: loco [-vhpdis] [-r <char>] [-R <string>] [file]\n");
   fprintf(stderr, "run 'loco -h' for more information\n");
}

void printVersion(void)
{
   printf("VERSION: 0.2\n");
}
/* >>> */

int main(int argc, char * const argv[])
{
   
   /* variables <<< */
   int  opt;
   FILE* input                           = stdin;
   bool nopipe                           = false;
   bool version                          = false;
   bool help                             = false;
   bool print                            = false;
   bool delete                           = false;
   bool skip                             = false;
   bool ignore                           = false;
   bool replaceC                         = false;
   bool replaceS                         = false;
   char ReplChar                         = '\0';
   char ReplString[MAX_REPLSTRING_LEN]   = {'\0'};
   int                     Character     = '\0';
   bool                    NewLine       = true;
   bool                    CommonEnds    = false;
   char                  **Lines         = NULL;
   char                   *Text          = NULL;
   unsigned long long int  MemNumOfLines = 0u;
   unsigned long long int  MemNumOfBytes = 0u;
   unsigned long long int  Line          = 0u;
   unsigned char           FirstLineDetect  = 0u;
   unsigned char           FirstLineProcess = 0u;
   unsigned long long int  Byte          = 0u;
   unsigned long long int  Column        = 0u;
   unsigned long long int  NumberOfLines = 0u;
   /* >>> */

   /* getopt <<< */
   while((opt = getopt(argc, argv, ":vhpdsir:R:")) != -1)
   {
      switch(opt)
      {
         case 'v':
            {
               version = true;
               break;
            }
         case 'h':
            {
               help = true;
               break;
            }
         case 'p':
            {
               print = true;
               break;
            }
         case 'd':
            {
               delete = true;
               break;
            }
         case 's':
            {
               skip = true;
               FirstLineProcess = 1u;
               break;
            }
         case 'i':
            {
               ignore = true;
               FirstLineDetect = 1u;
               FirstLineProcess = 1u;
               break;
            }
         case 'r':
            {
               ReplChar = optarg[0];
               replaceC = true;
               break;
            }
         case 'R':
            {
               strncpy(ReplString, optarg, MAX_REPLSTRING_LEN-1); /* strncpy does not append \0 */
               replaceS = true;
               break;
            }
         case ':':
            {
               if( optopt == 'r' )
               {
                  fprintf(stderr, "loco error: option -r (replace with character) needs a character value\n");
                  return 6;
               }
               else if( optopt == 'R' )
               {
                  fprintf(stderr, "loco error: option -R (replace with string) needs a string value\n");
                  return 7;
               }
               break;
            }
         case '?':
            {
               fprintf(stderr, "loco error: unknown option => '%c'\n", optopt);
               return 8;
               break;
            }
      }
   }

   if(version)
   {
      printVersion();
      return 0;
   }

   if(help || (print==false && delete==false && replaceC==false && replaceS==false))
   {
      printHelp();
      return 0;
   }

   if(isatty(fileno(stdin))) /* is connected to terminal, string not coming from pipe take second arg */
   {
      nopipe = true;

      if((argc - optind) < 1)
      {
         fprintf(stderr, "loco error: too few arguments\n");
         printUsage();
         return 2;
      }

      if((argc - optind) > 1)
      {
         fprintf(stderr, "loco error: too many arguments\n");
         printUsage();
         return 3;
      }

      input = fopen(argv[argc-1], "r");
      if(!input)
      {
         fprintf(stderr, "loco error: could not open file '%s'\n", argv[argc-1]);
         return 4;
      }
   }
   else /* program used in pipe take string from pipe */
   {
      if((argc - optind) > 0)
      {
         fprintf(stderr, "loco error: too many arguments\n");
         printUsage();
         return 5;
      }

      input = stdin;
   }

   if(skip && ignore)
   {
      FirstLineProcess = 2u;
   }

   if((print + delete + replaceS + replaceC) > 1) /* as C doesn't have a logical exclusive OR operator, I'm making use of true being defined as 1 */
   {
      fprintf(stderr, "loco error: the options -p -d -r -R are mutual exclusive. pick one.\n");
      printUsage();
      return 9;
   }
   /* >>> */

   /* read input <<< */
   while(true)
   {
      Character = fgetc(input);

      if(Byte == MemNumOfBytes)
      {
         MemNumOfBytes = MemNumOfBytes + BYTES_PER_MALLOC;
         Text = (char *)realloc(Text, MemNumOfBytes * sizeof(char));

         if(Text == NULL)
         {
            fprintf(stderr, "loco error: could not allocate memory\n");
            return 1;
         }
      }

      Text[Byte] = Character;

      if(Character == EOF) break;

      Byte++;
   }

   /* iterate bytes and create lines */
   Byte = 0u;
   while(true)
   {
      if(Line == MemNumOfLines)
      {
         MemNumOfLines = MemNumOfLines + LINES_PER_MALLOC;
         Lines = (char **)realloc(Lines, MemNumOfLines * sizeof(char *));
         if(Lines == NULL)
         {
            fprintf(stderr, "loco error: could not allocate memory\n");
            return 1;
         }
      }

      if(NewLine)
      {
         NewLine = false;
         Lines[Line] = &(Text[Byte]);
         Line++;
      }

      if(Text[Byte] == '\n')
      {
         Text[Byte] = '\0';

         if(Text[Byte+1u] != EOF)
         {
            NewLine = true;
         }
      }
      else if(Text[Byte] == EOF)
      {
         Text[Byte] = '\0';
         NumberOfLines = Line;
         break;
      }

      Byte++;
   }
   /* >>> */

   /* delete or replace common part <<< */
   if(delete || replaceS || replaceC)
   {
      while(true) /* iterate columns */
      {
         for(Line=FirstLineDetect ; Line < NumberOfLines ; Line++) /* iterate lines */
         {
            if(Line == FirstLineDetect)
            {
               Character = Lines[Line][Column];
            }

            if((Character != Lines[Line][Column]) || (Character == '\0'))
            {
               CommonEnds = true;
               break;
            }
         }

         if(CommonEnds == false)
         {
            for(Line=FirstLineProcess ; Line < NumberOfLines ; Line++) /* iterate lines */
            {
               if(replaceC)
               {
                  Lines[Line][Column] = ReplChar;
               }
            }
         }
         else
         {
            break;
         }

         Column++;
      }

      if(replaceS && (Column == 0))
      {
         ReplString[0] = '\0';
      }

      if(replaceC)
      {
         Column = 0;
      }

      for(Line=0 ; Line < NumberOfLines ; Line++)
      {
         if( ((Line == 0) && (ignore || skip)) || ((Line == 1) && (ignore && skip)) )
         {
            printf("%s\n", Lines[Line]);
            continue;
         }
         printf("%s%s\n", ReplString, &(Lines[Line][Column]));
      }
   }
   /* >>> */

   /* print common part <<< */
   if(print)
   {
      while(true) /* iterate columns */
      {
         for(Line=FirstLineDetect ; Line < NumberOfLines ; Line++) /* iterate lines */
         {
            if(Line == FirstLineDetect)
            {
               Character = Lines[Line][Column];
            }

            if((Character != Lines[Line][Column]) || (Character == '\0'))
            {
               CommonEnds = true;
            }
         }

         if(CommonEnds)
         {
            printf("\n");
            break;
         }
         else
         {
            printf("%c", Character);
         }

         Column++;
      }
   }
   /* >>> */

   /* clean up <<< */
   if(Lines != NULL)
   {
      free(Lines);
   }

   if(Text != NULL)
   {
      free(Text);
   }

   if(nopipe)
   {
      fclose(input);
   }
   /* >>> */

   return 0;
}

/* vim: fmr=<<<,>>> fdm=marker
 */
