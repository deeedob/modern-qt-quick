# Customize your application

## Rename the application

Run the customization script from the repository root:

```sh
python3 tools/make-it-yours/make_it_yours.py
```

It updates the project and application names. Review the resulting diff before building.

## Replace the application icon

Replace `src/app/icons/app.svg` with your artwork. Use a square SVG canvas and keep the
artwork readable at small sizes. Keep the filename so the existing build rules pick it up.
If you use third-party artwork, record its license in `REUSE.toml` and include any required
notice in the distributed application.

Rebuild and run:

```sh
cmake --build --preset dev
cmake --build --preset dev --target run
```

The same SVG supplies the icon shown inside the application. Native packaging handles it
as follows:

| Platform | Build output |
|---|---|
| macOS | The icon generator renders PNG sizes, `iconutil` creates `app.icns`, and CMake copies it into the app bundle's `Contents/Resources`. |
| Windows | The icon generator creates `app.ico`, which the resource script embeds in the executable. |
| Linux | The current install rules do not install a desktop launcher or desktop icon. Add these when choosing a Linux desktop packaging format. |

macOS caches application icons. The build refreshes the bundle timestamp when it relinks,
but an already running app may still show its previous Dock icon. Quit the app, rebuild,
and launch the rebuilt bundle again. Check the bundle from the current build directory
rather than an older installed copy.

When cross-compiling for macOS or Windows, the build needs a host-built
`app_icon_generator`. Set the `APP_ICON_GENERATOR` CMake cache entry to that executable;
a target-platform executable cannot run on the build host.
