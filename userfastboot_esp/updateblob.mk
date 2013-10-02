UFP_ESP_UPDATE_MK_BLOB := vendor/intel/recovery_plugins/userfastboot_esp/mkbblob.py
UFP_ESP_UPDATE_FILES := $(filter %.efi,$(INSTALLED_RADIOIMAGE_TARGET))
UFP_ESP_UPDATE_BLOB := $(OUT)/ufp_esp_update.bin

$(UFP_ESP_UPDATE_BLOB): \
		$(UFP_ESP_UPDATE_FILES) \
		$(UFP_ESP_UPDATE_MK_BLOB)
	$(hide) mkdir -p $(dir $@)
	$(hide) $(UFP_ESP_UPDATE_MK_BLOB) \
			--output $@ \
			$(UFP_ESP_UPDATE_FILES)

droidcore: $(UFP_ESP_UPDATE_BLOB)
$(call dist-for-goals,droidcore,$(UFP_ESP_UPDATE_BLOB):$(TARGET_PRODUCT)-ufp_esp_update-$(FILE_NAME_TAG).bin)

