#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#define LEVENSHTEIN_MAX_LENGTH 512

/* Searching for string a in string b
 * returns the edit distance of substring of b with the lowest
 * edit distance ( <= max_distance ) from a 
 * or max_distance -1, if no such substring is found
 */
static inline unsigned levenshtein_substring_match(
	const char *a, unsigned length_a,
	const char *b, unsigned length_b,
	unsigned max_distance)
{
	(void)a;
	(void)b;
	(void)length_a;
	(void)length_b;
	(void)max_distance;
	return 0;
}

#endif /* LEVENSHTEIN_H */
