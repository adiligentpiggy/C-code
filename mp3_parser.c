#include <stdio.h>
#include<fcntl.h>
#define ERROR printf
#   define assert(x)	/* nothing */
typedef unsigned char id3_byte_t;
typedef unsigned long id3_length_t;

typedef unsigned long id3_ucs4_t;

typedef unsigned char id3_latin1_t;
typedef unsigned short id3_utf16_t;
typedef signed char id3_utf8_t;
enum {
  ID3_TAG_FLAG_UNSYNCHRONISATION     = 0x80,
  ID3_TAG_FLAG_EXTENDEDHEADER        = 0x40,
  ID3_TAG_FLAG_EXPERIMENTALINDICATOR = 0x20,
  ID3_TAG_FLAG_FOOTERPRESENT         = 0x10,

  ID3_TAG_FLAG_KNOWNFLAGS            = 0xf0
};
static void print_buf(char * p_buf, int len)
{
    int i = 0;

    printf("debug client buf is:\r\n");
    for(; i < len; i++)
    {
        printf("%02X ", p_buf[i] & 0xff);
    }
    printf("\r\n");
}

enum tagtype {
  TAGTYPE_NONE = 0,
  TAGTYPE_ID3V1,
  TAGTYPE_ID3V2,
  TAGTYPE_ID3V2_FOOTER
};

unsigned long id3_parse_uint(id3_byte_t const **ptr, unsigned int bytes)
{
  unsigned long value = 0;

  assert(bytes >= 1 && bytes <= 4);

  switch (bytes) {
  case 4: value = (value << 8) | *(*ptr)++;
  case 3: value = (value << 8) | *(*ptr)++;
  case 2: value = (value << 8) | *(*ptr)++;
  case 1: value = (value << 8) | *(*ptr)++;
  }

  return value;
}

unsigned long id3_parse_syncsafe(id3_byte_t const **ptr, unsigned int bytes)
{
  unsigned long value = 0;

  assert(bytes == 4 || bytes == 5);

  switch (bytes) {
  case 5: value = (value << 4) | (*(*ptr)++ & 0x0f);
  case 4: value = (value << 7) | (*(*ptr)++ & 0x7f);
          value = (value << 7) | (*(*ptr)++ & 0x7f);
	  value = (value << 7) | (*(*ptr)++ & 0x7f);
	  value = (value << 7) | (*(*ptr)++ & 0x7f);
  }

  return value;
}

static
void parse_header(id3_byte_t const **ptr,
		  unsigned int *version, int *flags, id3_length_t *size)
{
  *ptr += 3;

  *version = id3_parse_uint(ptr, 2);
  *flags   = id3_parse_uint(ptr, 1);
  *size    = id3_parse_syncsafe(ptr, 4);
}

static
enum tagtype tagtype(id3_byte_t const *data, id3_length_t length)
{
  if (length >= 3 &&
      data[0] == 'T' && data[1] == 'A' && data[2] == 'G')
    return TAGTYPE_ID3V1;

  if (length >= 10 &&
      ((data[0] == 'I' && data[1] == 'D' && data[2] == '3') ||
       (data[0] == '3' && data[1] == 'D' && data[2] == 'I')) &&
      data[3] < 0xff && data[4] < 0xff &&
      data[6] < 0x80 && data[7] < 0x80 && data[8] < 0x80 && data[9] < 0x80)
    return data[0] == 'I' ? TAGTYPE_ID3V2 : TAGTYPE_ID3V2_FOOTER;

  return TAGTYPE_NONE;
}

signed long id3_tag_query(id3_byte_t const *data, id3_length_t length)
{
  unsigned int version;
  int flags;
  id3_length_t size;

  assert(data);

  switch (tagtype(data, length)) {
  case TAGTYPE_ID3V1:
    return 128;

  case TAGTYPE_ID3V2:
    parse_header(&data, &version, &flags, &size);

    if (flags & ID3_TAG_FLAG_FOOTERPRESENT)
      size += 10;

    return 10 + size;

  case TAGTYPE_ID3V2_FOOTER:
    parse_header(&data, &version, &flags, &size);
    return -size - 10;

  case TAGTYPE_NONE:
    break;
  }

  return 0;
}

int play_mp3_file(char *path) {
	char *buff = NULL;
    int buf_size = 1152;
    int buf_len = 0;
    int count;
	if (path == NULL) {
      ERROR("path == NULL");
      return -1;
  }
  int fd;
  fd = open(path, O_RDWR);
  if(fd < 0)
  {
    ERROR("open file failed:%s", path);
    return -1;
  }
  /*if (fstat(fd, &stat) == -1 ||stat.st_size == 0)
  {
    ERROR("fstat failed");
    goto Err;
  }*/
  
  buff = malloc(buf_size);
  memset(buff, 0, buf_size);
  
  if((count = read(fd, buff, buf_size)) > 0){
  	if (strncmp(buff, "ID3", 3) == 0) {
  		unsigned char *Size = &buff[6]; 
  		int size=(Size[0]&0x7F)*0x200000+(Size[1]&0x7F)*0x400+(Size[2]&0x7F)*0x80+(Size[3]&0x7F);
  		printf("tag size is %x %x %x %x %d\n",
  		(Size[0]&0x7F),
  		(Size[1]&0x7F),
  		(Size[2]&0x7F),
  		(Size[3]&0x7F),
  		 size);
  		print_buf(buff+size, 4);
  		
  	}
  		
  }
  
  signed long tagsize = id3_tag_query(buff, buf_size);
  printf("tagsize %d\n", tagsize);
  print_buf(&buff[tagsize], 4);
}

int main(int argc, char *argv[]) {
	printf("mp3 file name is %s\n", argv[1]);
	play_mp3_file(argv[1]);
	return 0;
}