ODIR=obj
LIBS=-lcurl -lpthread
CFLAGS += "-Werror"

# The name of the executable to be created
BIN_NAME := ota-update
OTA_INIT_FILE := ota-update-init

# Programs for installation
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m 755

# Install path (bin/ is appended automatically)
INSTALL_PREFIX = /usr/bin

SYSCONFDIR=/etc

DEPS = ota_download.h ota_update.h jsmn.h ota_utils.h ota_indicator.h

#_OBJ = ota_app.o jsmn.o ota_download.o ota_update.o ota_utils.o ota_indicator.o
_OBJ = main.o jsmn.o ota_download.o ota_update.o ota_utils.o ota_indicator.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

ota-update: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) ${LDFLAGS}


# Installs to the set path
.PHONY: install
install:
	mkdir -p $(INSTALL_PREFIX) ${SYSCONFDIR}/init.d/
	@echo "Installing $(BIN_NAME) to $(INSTALL_PREFIX)"
	@$(INSTALL_PROGRAM) $(BIN_NAME) $(INSTALL_PREFIX)
	@echo "Installing $(OTA_INIT_FILE) to ${SYSCONFDIR}/init.d/"
	@$(INSTALL_PROGRAM) $(OTA_INIT_FILE) ${SYSCONFDIR}/init.d/

# Uninstalls the program
.PHONY: uninstall
uninstall:
	@echo "Removing $(INSTALL_PREFIX)/$(BIN_NAME)"
	@$(RM) $(INSTALL_PREFIX)/$(BIN_NAME)
	@echo "Removing ${SYSCONFDIR}/init.d/$(OTA_INIT_FILE)"
	@$(RM) ${SYSCONFDIR}/init.d/$(OTA_INIT_FILE)

.PHONY: clean

clean:
	rm -rf $(ODIR)
