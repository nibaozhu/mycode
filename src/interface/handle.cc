/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"

/*
 * %t: ....
 */
int handle(Transport *t, std::map<int, Transport*> *m, std::queue<Transport*> *w) {
	int ret = 0;
	unsigned int length = 0;
	int width = 0;
	char md5sum[MD5SUM_LENGTH + 1];
	char digestname[] = "md5";
	char id0[ID_LENGTH + 1];
	char id1[ID_LENGTH + 1];
	memset(id0, 0, sizeof id0);
	memset(id1, 0, sizeof id1);
	int i = 0;
	char c = 0;
	std::string id;
	static std::map<std::string, int> *interface = new std::map<std::string, int>(); // maybe should use multimap
	void *message = NULL;

	printf("rx = %p, rp = 0x%lx\n", t->get_rx(), t->get_rp());
	t->pr();
	do {

		if (t->get_rp() >= LENGTH) {
			for (i = 0; i < LENGTH; i++) {
				c = *((char*)t->get_rx() + i);
				if (c >= '0' && c <= '9') {
					length = length * 0x10 + (c - '0' + 0x00);
				} else if (c >= 'a' && c <= 'f') {
					length = length * 0x10 + (c - 'a' + 0x0a);
				} else if (c >= 'A' && c <= 'F') {
					length = length * 0x10 + (c - 'A' + 0x0a);
				}
			}
			printf("i8.length = 0x%08x\n", length);
			width += LENGTH;
		} else {
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + ID_LENGTH) {
			strncpy(id0, (char*)t->get_rx() + width, ID_LENGTH);
			printf("id0 = \"%s\", id = \"%s\"\n", id0, t->get_id().c_str());
			width += ID_LENGTH;
		} else {
			/* Back to wait message. */
			/* Wait message */
			break;
		}

		if (t->get_rp() >= width + ID_LENGTH) {
			strncpy(id1, (char*)t->get_rx() + width, ID_LENGTH);
			printf("id1 = \"%s\", id = \"%s\"\n", id1, t->get_id().c_str());
			width += ID_LENGTH;
		} else {
			/* Wait message */
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + MD5SUM_LENGTH) {
			memset(md5sum, 0, sizeof md5sum);
			memcpy(md5sum, (const void *)((char *)t->get_rx() + width), MD5SUM_LENGTH);
			printf("i2.md5sum = \"%s\"\n", md5sum);
			width += MD5SUM_LENGTH;
		} else {
			/* Wait message */
			/* Back to wait message. */
			break;
		}

		if (t->get_rp() >= width + length) {
			message = malloc(length + 1);
			memset(message, 0, sizeof message);
			memcpy(message, (const void *)((char *)t->get_rx() + width), length);
			ret = checksum(message, length, md5sum, digestname);
			if (ret == -1) {
				printf("checksum FAIL\n");

				t->clear_rx();
				/* Back to wait message. */
				break;
			}
		}

		if (strncmp(id0, id1, sizeof id0) == 0 && strlen(id0) > 0) {
			printf("Echo.\n");
			interface->erase(t->get_id());
			t->set_id(id0);
			(*interface)[t->get_id()] = t->get_fd();
			t->set_id(id0);
			if (t->get_rp() >= width) {
				t->set_wx(t->get_rx(), t->get_rp());
				t->clear_rx();
				w->push(t);
			} else {
				break;
			}
		} else if (t->get_rp() >= width + length) {
			printf("Message complete, width + length = 0x%lx, rp = 0x%lx\n", width + length, t->get_rp());
			id = id1;
			Transport *t2 = (*m)[(*interface)[id]];
			if (t2 == NULL) {
				printf("Back to wait id = \"%s\"\n", id1);
				break;
			}

			if (t->get_rp() >= width) {
				t2->set_wx(t->get_rx(), (width + length));
				memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (width + length)), t->get_rp() - (width + length));
				t->set_rp(t->get_rp() - (width + length));
				w->push(t2);
			} else {
				break;
			}
		}
	} while (0);
	return ret;
}

int checksum(const void *ptr, int size, char *md_value_0, char *digestname) {
	int ret = 0;
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	char md_value_x;
	int md_len, i;

	OpenSSL_add_all_digests();
	md = EVP_get_digestbyname(digestname);
	if (!md) {
		printf("Unknown message digest %s\n", digestname);
		return -1;
	}

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, ptr, size);
	EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *)&md_len);
	EVP_MD_CTX_destroy(mdctx);

#if 0
	for (i = 0; i < size; i++) printf("%c", *(char *)((char *)ptr + i));
	puts("");

	puts("Original Digest is: {");
	for (i = 0; i < strlen(md_value_0); i++) printf("%c", md_value_0[i]);
	puts("}");

	puts("Computed Digest is: {");
	for (i = 0; i < md_len; i++) printf("%02x", md_value[i]);
	puts("}");
#endif

	for (i = 0; i < md_len; i++) {

		if (md_value_0[2 * i] >= '0' && md_value_0[2 * i] <= '9') {
			md_value_0[2 * i] = md_value_0[2 * i] - '0';
		} else if (md_value_0[2 * i] >= 'a' && md_value_0[2 * i] <= 'f') {
			md_value_0[2 * i] = md_value_0[2 * i] - 'a' + 0x0a;
		} else if (md_value_0[2 * i] >= 'A' && md_value_0[2 * i] <= 'F') {
			md_value_0[2 * i] = md_value_0[2 * i] - 'A' + 0x0a;
		}

		if (md_value_0[2 * i + 1] >= '0' && md_value_0[2 * i + 1] <= '9') {
			md_value_0[2 * i + 1] = md_value_0[2 * i + 1] - '0';
		} else if (md_value_0[2 * i + 1] >= 'a' && md_value_0[2 * i + 1] <= 'f') {
			md_value_0[2 * i + 1] = md_value_0[2 * i + 1] - 'a' + 0x0a;
		} else if (md_value_0[2 * i + 1] >= 'A' && md_value_0[2 * i + 1] <= 'F') {
			md_value_0[2 * i + 1] = md_value_0[2 * i + 1] - 'A' + 0x0a;
		}

		if (md_value[i] != md_value_0[2 * i] * 0x10 + md_value_0[2 * i + 1]) {
			puts("Warning: CHECKSUM NOT EQUAL!\n");
			ret = -1;
			break;
		}
	}
	return ret;
}
