# Plugin example

This is an example on how to create a wheel of a plugin for Blosc2.

## Create the wheel

```shell
python setup.py bdist_wheel
```

## Verify the wheel is working

```shell
pip install dist/blosc2_btune_plugin-*.whl
```

And then simulate that you are going to uninstall.  You will be presented where the plugin has been installed:

```shell
pip uninstall blosc2_btune_plugin
```

For example, the shared library should appear somewhere like:

```shell
ls -l /Users/faltet/miniconda3/lib/python3.9/site-packages/blosc2_btune_plugin
total 112
-rw-r--r--  1 faltet  staff   258B Mar  6 13:45 __init__.py
drwxr-xr-x  3 faltet  staff    96B Mar  6 13:45 __pycache__/
-rwxr-xr-x  1 faltet  staff    48K Mar  6 13:45 libblosc2_btune_plugin.so*
```

In the future, you should be able to test that the wheel is working with this command:

```shell
blosc2_test_plugin plugin_example_name
```

That's all folks!
