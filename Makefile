CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = furnish
SRC = furnish.c

# Build the converter
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Usage: make run IN=path/to/file.c OUT=path/to/file.html
run: $(TARGET)
	@if [ -z "$(IN)" ]; then \
		echo "Usage: make run IN=path/to/file.c [OUT=path/to/file.html]"; \
	else \
		sh -c '\
			if [ -z "$(OUT)" ]; then \
				BASE=$$(basename "$(IN)" .c); \
				DIR=$$(dirname "$(IN)"); \
				if [ "$$DIR" = "." ]; then OUT="$$BASE.html"; else OUT="$$DIR/$$BASE.html"; fi; \
			fi; \
			./$(TARGET) "$(IN)" "$$OUT"; \
		'; \
	fi

# Run on all .C files in a directory (set DIR variable)
# Example: make runall DIR=./trial/pages/
runall: $(TARGET)
	@if [ -z "$(DIR)" ]; then \
		echo "Usage: make runall DIR=path/to/directory"; \
	else \
		for file in $(DIR)/*.c; do \
			BASE=$$(basename "$$file" .c); \
			OUT="$(DIR)/$$BASE.html"; \
			echo "Processing $$file -> $$OUT"; \
			./$(TARGET) "$$file" "$$OUT"; \
		done \
	fi

# Clean generated files and executable
clean:
	rm -f $(TARGET) *.html *.cfg */*.html */*.cfg
