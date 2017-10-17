# pebble-fit
Become less sedentary with pebble

# Getting Started (Local Development)
1. Install Pebble SDK.
2. Get source code.
`git clone https://github.com/snap-stanford/pebble-fit.git`
3. Build the app for the first time. The `node_modules` directory will be created.
`pebble build`
4. Apply patch. Move pebble-fit/enamel.c.jinja to pebble-fit/node_modules/enamel/templates/ and overwrite the one exists in that directory.
`mv -f enamel.c.jinja node_modules/enamel/templates/`
5. Rebuild app.
`pebble build`

Note: we have depricated building this app with CloudPebble.
