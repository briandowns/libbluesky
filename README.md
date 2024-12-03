# libbluesky

[![Build and Deploy](https://github.com/briandowns/libbluesky/actions/workflows/main.yml/badge.svg)](https://github.com/briandowns/libbluesky/actions/workflows/main.yml/badge.svg)

libbluesky is a C/C++ client library for the BlueSky API. It requires a Bluesky account to connect through and an application password to authenticate with.

## Usage

To initialize the library, the user's github token is required.

```c
bs_client_init(handle, app_password);
```

## Build shared object

To build the shared object:

```sh
make install
```

## Example 

Build the example:

```sh
make example
```
## API Coverage

* API response data is returned in a string containing JSON.
* The caller is responsible for how to handle the data.

### asdf

## Requirements / Dependencies

* curl
* jansson

## Contributing

Please feel free to open a PR!

## Contact

Brian Downs [@bdowns328](http://twitter.com/bdowns328)

## License

BSD 2 Clause [License](/LICENSE).
