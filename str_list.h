#ifndef STR_LIST_H
#define STR_LIST_H

struct str_list {
	char *str;
	struct str_list *next;
};

struct str_list	*str_list_new();
void			 str_list_print(struct str_list *);
void			 str_list_free(struct str_list *);

#endif /* STR_LIST_H */
