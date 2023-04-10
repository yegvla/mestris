TILE_MAP = '{"000000":0,"ffffff":1,"ff0000":2,"00ff00":3,"0000ff":4,"ffff00":5,"ff00ff":6,"00ffff":7}'

assets/TILE_%.m3if: $(wildcard tiles/*.png)
	for file in $^ ; do \
		base=$$(basename $${file} .png) ; \
		cd .mes/gpu/image2mes; cargo run -- -i ../../../$${file} -c $(TILE_MAP) -t bin -o ../../../assets/$${base}.m3if ; \
	done

tiles: assets/TILE_%.m3if

simulate: tiles
	mvm simulate

flash: tiles
	mvm flash

iso: tiles
	cd .mbs; make iso