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
   printf("USAGE: loco [-vhpds] [-r <char>] [-R <string>] [file]\n");
   printf("\n");
   printf("OPTIONS:\n");
   printf("  -p          print longest common string\n");
   printf("  -d          print without longest common string\n");
   printf("  -s          skip first line\n");
   printf("  -r <char>   replace each character of longest common string with <char>\n");
   printf("  -R <string> replace longest common string with <string>\n");
   printf("  -v          print version info\n");
   printf("  -h          print help text\n");
   printf("\n");
}

void printUsage(void)
{
   fprintf(stderr, "USAGE: loco [-vhpds] [-r <char>] [-R <string>] [file]\n");
   fprintf(stderr, "run 'loco -h' for more information\n");
}

void printVersion(void)
{
   printf("VERSION: 0.1\n");
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
   unsigned char           FirstLine     = 0u;
   unsigned long long int  Byte          = 0u;
   unsigned long long int  Column        = 0u;
   unsigned long long int  NumberOfLines = 0u;
   /* >>> */

   /* getopt <<< */
   while((opt = getopt(argc, argv, ":vhpdsr:R:")) != -1)
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
               FirstLine = 1u;
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
               return 1;
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
   /* >>> */

   /* read input <<< */
   while((Character = fgetc(input)) != EOF)
   {
      if(Line == MemNumOfLines)
      {
         MemNumOfLines = MemNumOfLines + LINES_PER_MALLOC;
         Lines = (char **)realloc(Lines, MemNumOfLines * sizeof(char *));
         if(Lines == NULL)
         {
            /* TODO handle error */
         }
      }

      if(Byte == MemNumOfBytes)
      {
         MemNumOfBytes = MemNumOfBytes + BYTES_PER_MALLOC;
         Text = (char *)realloc(Text, MemNumOfBytes * sizeof(char));

         if(Text == NULL)
         {
            /* TODO handle error */
         }
      }

      if(NewLine)
      {
         NewLine = false;
         Lines[Line] = &(Text[Byte]);
         Line++;
      }

      if(Character == '\n')
      {
         NewLine = true;
         Text[Byte] = '\0';
      }
      else
      {
         Text[Byte] = Character;
      }

      Byte++;
   }
   Text[Byte]  = '\0';
   Lines[Line] = NULL;
   NumberOfLines = Line;
   /* >>> */

   /* delete or replace common part <<< */
   if(delete || replaceS || replaceC)
   {
      while(true) /* iterate columns */
      {
         for(Line=0u ; Line < NumberOfLines ; Line++) /* iterate lines */
         {
            if(Line == 0u)
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
            for(Line=0u ; Line < NumberOfLines ; Line++) /* iterate lines */
            {
               if(replaceC)
               {
                  Lines[Line][Column] = ReplChar;
               }
               else
               {
                  (Lines[Line])++;
               }
            }
         }
         else
         {
            break;
         }

         if(replaceC)
         {
            Column++;
         }
      }

      if(skip)
      {
         printf("%s\n", Text); /* okay, I admit this is a bit of a dirty hack */
      }

      for(Line=FirstLine ; Line < NumberOfLines ; Line++)
      {
         printf("%s%s\n", ReplString, Lines[Line]);
      }
   }
   /* >>> */

   /* print common part <<< */
   if(print)
   {
      while(true) /* iterate columns */
      {
         for(Line=0u ; Line < NumberOfLines ; Line++) /* iterate lines */
         {
            if(Line == 0u)
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
