OBJ := a.out

SRCS += weather.c
SRCS += cJSON.c

$(OBJ):$(SRCS)
	gcc $^ -o $@

.PHONY:
clean:
	rm $(OBJ)