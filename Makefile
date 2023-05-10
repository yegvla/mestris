TEXTURE_MAP = '{"000000":0,"ffffff":1,"ff0000":2,"00ff00":3,"0000ff":4,"ffff00":5,"ff00ff":6,"00ffff":7}'

assets/%.m3if: $(wildcard textures/*.png)
	for file in $^ ; do \
		base=$$(basename $${file} .png) ; \
		cd .mes/gpu/image2mes; cargo run -- -i ../../../$${file} -c $(TEXTURE_MAP) -t bin -o ../../../assets/$${base}.m3if ; \
	done

textures: assets/%.m3if

simulate: textures
	mvm simulate

flash: textures
	mvm flash

iso: textures
	cd .mbs; make iso
