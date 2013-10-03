UFB_ESP_UPDATE_MK_BLOB := vendor/intel/recovery_plugins/userfastboot_esp/mkbblob.py
UFB_ESP_UPDATE_FILES := $(filter %.efi,$(INSTALLED_RADIOIMAGE_TARGET))
UFB_ESP_UPDATE_BLOB := $(OUT)/ufb_esp_update.bin

$(UFB_ESP_UPDATE_BLOB): \
		$(UFB_ESP_UPDATE_FILES) \
		$(UFB_ESP_UPDATE_MK_BLOB)
	$(hide) mkdir -p $(dir $@)
	$(hide) $(UFB_ESP_UPDATE_MK_BLOB) \
			--output $@ \
			$(UFB_ESP_UPDATE_FILES)

droidcore: $(UFB_ESP_UPDATE_BLOB)
$(call dist-for-goals,droidcore,$(UFB_ESP_UPDATE_BLOB):$(TARGET_PRODUCT)-ufb_esp_update-$(FILE_NAME_TAG).bin)

