# (1) Base URL

`https://wakatime.com`

# (2) Authorization

## OAuth

Can create an OAuth app and have users sign in

This will necessitate a backend service where users can sign in with WT,
and the display can then fetch from there

## API Keys

WT also supports personal API keys unique to each user. This can be used
without the need for a backend service; the key can be entered
straight into the firmware, similarly to the WiFi credentials.

## Passing auth to API requests

- We can use `api_key` as a query parameter
- We can pass the token (base64 encoded) with HTTP Basic Auth (`Authorization: Basic <b64>`)

# (3) Fetching data to display

## Status Bar API

The Status Bar API seems the most appropriate for the goals of the project;
it's what's used for the status bars in IDEs as well.

```
{
  "data": {
    "grand_total": {
      "decimal":"",
      "digital":"",
      "hours":0,
      "minutes":0,
      "text":"",
      "total_seconds":0
    },
    "categories":[],
    "dependencies":[],
    "editors":[],
    "languages":[],
    "machines":[],
    "operating_systems":[],
    "projects":[],
    "range": {
      "text":"Today",
      "timezone":"UTC"
    }
  }
}
```
