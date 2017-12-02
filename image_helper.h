#ifndef __IMAGE_HELPER_H__
#define __IMAGE_HELPER_H__

#include "csapp.h"

void send_image(uint32_t fd, char *buf, size_t len, uint32_t toConnfd);
void recv_image(uint32_t fd, char *buf, size_t len, uint32_t fromConnfd, uint32_t fileSize);

#endif /*__IMAGE_HELPER_H__*/