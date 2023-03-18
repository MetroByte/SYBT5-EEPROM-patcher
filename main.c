#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
  unsigned char inbuffer[512];  /* from file reading*/
  unsigned char outbuffer[512]; /* data changes go here */
  FILE *fptr;                   /* input file (argv[1]) */
  long insize; /* for checking input file size (must be 512 bytes) */
  int i;       /* loop iterator */
  unsigned short original_checksum = 0; /* 2 chars into short*/
  unsigned short real_checksum = 0;     /* sum of first 160 bytes */
  char answer;                          /* Y/N */
  /* Time and date */
  time_t t = time(NULL);         /* Time for S/N and Date change */
  struct tm *tm = localtime(&t); /* Local time struct */
  char week_char[11];
  char year_char[11];
  char day_char[11];
  char month_char[11];
  short monday; /* for week number calculation */
  unsigned char current_month;
  short monday_year;
  short monday_first;
  short current_day;
  short current_week;

  char *file_name_w_ext;     /* file name + ext from argv[1] */
  char *filename_no_ext;     /* file name without extension */
  char *extension;           /* input file extension */
  char result_filename[256]; /* output file name buffer */
  FILE *write_ptr;           /* output file */

  /* read EEPROM dump file */
  printf("APC Symmetra SYBT5 EEPROM patcher, 2023\n\n");
  if (argc < 2) {
    printf("You have to specify the EEPROM dump file with commandline argument "
           "or drag-and-drop it on this executable.\n");
    getchar();
    exit(1);
  }
  fptr = fopen(argv[1], "rb");
  if (fptr == NULL) {
    printf("Error: can't open file %s.\n", argv[1]);
    getchar();
    exit(1);
  }
  fseek(fptr, 0, SEEK_END);

  insize = ftell(fptr);
  if (insize != 512) {
    printf("Error: invalid size. Expected 512 bytes, got %lu.\n", insize);
    getchar();
    exit(1);
  }
  fseek(fptr, 0, SEEK_SET);
  fread(inbuffer, sizeof(inbuffer), 1, fptr);

  /* Check S Y B T 5 */
  if (inbuffer[120] != 'S' || inbuffer[122] != 'Y' || inbuffer[124] != 'B' || inbuffer[126] != 'T' || inbuffer[128] != '5') {
    printf("Error: EEPROM binary %s has incorrect data. Expected SYBT5, got %c%c%c%c%c", argv[1], inbuffer[120], inbuffer[122], inbuffer[124],inbuffer[126],inbuffer[128]);
    getchar();
    exit(1);
  }

  /* Read original data */
  printf("Original serial: ");
  for (i = 0; i < 12; i++) {
    printf("%c", inbuffer[i * 2 + 88]);
  }
  printf("\nOriginal date: ");
  for (i = 0; i < 8; i++) {
    printf("%c", inbuffer[i * 2 + 144]);
  }

  /* Read present checksum */
  original_checksum += inbuffer[160];
  original_checksum += inbuffer[162] << 8;
  printf("\nOriginal checksum: %02X%02X\n", inbuffer[160], inbuffer[162]);
  printf("\n======\n\n");

  /* Calc real checksum */
  for (i = 0; i < 160; i++) {
    real_checksum += inbuffer[i];
  }

  if (original_checksum != real_checksum) {
    printf("== WARN: wrong checksum: in: %i, real: %i ==\n\n",
           original_checksum, real_checksum);
  }

  /* copy input EEPROM dump to outbuffer*/
  memcpy(outbuffer, &inbuffer, 512);

  /* reset errors */
  for (i = 163; i < 512; i++) {
    outbuffer[i] = 0;
  }

  /* ask for S/N + Date rewrite */
  printf("Change serial + date?\n[y/N]\n");

  answer = getchar();

  if (answer == 'y' || answer == 'Y') {
    /* New date
    getting week number */
    current_day = tm->tm_yday;
    monday = current_day - (tm->tm_wday + 6) % 7;
    monday_year = 1 + (monday + 6) % 7;
    monday_first = (monday_year > 4) ? monday_year - 7 : monday_year;
    current_week = 1 + (monday - monday_first) / 7;
    sprintf(week_char, "%02d", current_week);

    sprintf(year_char, "%02d", tm->tm_year % 100);
    sprintf(day_char, "%02d", tm->tm_mday);
    current_month = tm->tm_mon+1;
    sprintf(month_char, "%02d", current_month);

    /* write new date */
    outbuffer[144] = month_char[0];
    outbuffer[146] = month_char[1];
    outbuffer[150] = day_char[0];
    outbuffer[152] = day_char[1];
    outbuffer[156] = year_char[0];
    outbuffer[158] = year_char[1];

    /* write new S/N */
    outbuffer[92] = year_char[0];
    outbuffer[94] = year_char[1];
    outbuffer[96] = week_char[0];
    outbuffer[98] = week_char[1];

    /* Calculate new checksum */
    real_checksum = 0;
    for (i = 0; i < 160; i++) {
      real_checksum += outbuffer[i];
    }
    outbuffer[162] = ((real_checksum >> (8 * 1)) & 0xff);
    outbuffer[160] = (real_checksum & 0xff);

    printf("\nNew data:\n");
    printf("Serial: ");
    for (i = 0; i < 12; i++) {
      printf("%c", outbuffer[i * 2 + 88]);
    }

    printf("\nDate: ");
    for (i = 0; i < 8; i++) {
      printf("%c", outbuffer[i * 2 + 144]);
    }

    printf("\nChecksum: %02X%02X\n", outbuffer[160], outbuffer[162]);
  }

  /* Get filename from cmdline args */
  file_name_w_ext = argv[1] + strlen(argv[1]);
  for (; file_name_w_ext > argv[1]; file_name_w_ext--) {
    if ((*file_name_w_ext == '\\') || (*file_name_w_ext == '/')) {
      file_name_w_ext++;
      break;
    }
  }

  filename_no_ext = malloc(strlen(file_name_w_ext) + 1);
  strcpy(filename_no_ext, file_name_w_ext);
  extension = strrchr(filename_no_ext, '.');
  if (extension != NULL) {
    *extension = '\0';
  }

  sprintf(result_filename, "%s-fix.bin", filename_no_ext);

  /* write to file */
  write_ptr = fopen(result_filename, "wb");
  fwrite(outbuffer, sizeof(outbuffer), 1, write_ptr);

  printf("\nDone.\nPatched file: %s\n=====\n", result_filename);

  getchar();
  return 0;
}
