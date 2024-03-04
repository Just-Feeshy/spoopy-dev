<xml>
    <files id="spoopy-toolkit-volk-depends">
	<depend name="lib/volk/volk.h" />
    </files>

    <files id="spoopy-toolkit-volk" tags="">
        <depend files="spoopy-toolkit-volk-depends" />
	<compilerflag value="-IVulkanSDK/include" />

        <file name="lib/volk/volk.c" />
    </files>
</xml>
