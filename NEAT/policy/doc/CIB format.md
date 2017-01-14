# CIB format

The CIB is a repository containing a list of potential candidates for new connections.

Each entry or `row` of the CIB is comprised of an arbitrary number of NEAT Properties. Rows are composed using JSON encoded *CIB files* stored in the CIB directory - these files are generated by different *CIB sources*. A simple example of the JSON file defining a local interface is given below:

```
{
    "uid": "eth0",
    "root": true,
    "priority": 4,
    "properties": {
        "interface": {"value": "eth0", "precedence":2},
        "capacity": {"value": 10000, "precedence":2},
        "local_ip": {"value": "10.10.2.1", "precedence":2},
        "is_wired": {"value": true, "precedence":2},
        "MTU": {"value": {"start":50, "end":9000}}
    }
}
```

Each CIB file includes a `uid` key and a set of `properties` to be included in the CIB row. 


## Extending Existing CIB Files

Additionally, CIB sources may generate CIB files which reference and extend already existing **CIB files**. This feature is used to inject network characteristics collected by external sources (e.g., controllers). For example, the following two CIB files reference the file above:

```
{
    "uid": "eth0_remote_1",
    "description": "information about remote endpoint 1",
    "priority": 2,
    "link": true,
    "match" : [
    	{"uid": {"value": "eth0"}}
    ],
    "properties": {
        "remote_ip": {"value": "8.8.8.8", "precedence":2, "score": 2},
        "remote_port": {"value": "80", "precedence":2, "score": 1}
    }
}
```
and

```
{
    "uid": "eth0_remote_2",
    "description": "information about remote endpoint 2",
    "priority": 2,
    "link": true,
    "match" : [
    	{"interface": {"value": "eth0"}, "local_ip": {"value": "10.10.2.1"}}
    ],
    "properties": {
        "remote_ip": {"value": "8.8.4.4.", "precedence":2, "score": 2},
    }
}
```


If a CIB file contains a `link` attribute that is set to `true` the CIB will attempt to match any property or `uid` listed in the `match` attribute, and will append the new properties to the corresponding CIB file. Both examples above will be linked to the interface `eth0`, defined in the first CIB file. To reference multiple CIBs, the match attribute list can contain multiple JSON objects. The priority attribute is used to resolve overlapping properties.

Essentially, the CIB internally constructs a directed graph using all available CIB files. To generate the CIB rows, the graph is resolved starting at each node which has the attribute `root` set to `true` (see CIB file 1), generating paths (i.e., rows) by traversing the graph in the reverse direction of the edges (TODO clarify).

Hence, from the above CIB files the CIB will generate the following two rows:

```
1: {"interface": {"value": "eth0", "precedence":2}, "capacity": {"value": 10000, "precedence":2}, "local_ip": {"value": "10.10.2.1", "precedence":2}, "is_wired": {"value": true, "precedence":2}, "MTU": {"value": {"start":50, "end":9000}, "remote_ip": {"value": "8.8.8.8", "precedence":2, "score": 2}, "remote_port": {"value": "80", "precedence":2, "score": 1}}

2: {"interface": {"value": "eth0", "precedence":2}, "capacity": {"value": 10000, "precedence":2}, "local_ip": {"value": "10.10.2.1", "precedence":2}, "is_wired": {"value": true, "precedence":2}, "MTU": {"value": {"start":50, "end":9000}, "remote_ip": {"value": "8.8.4.4.", "precedence":2, "score": 2}}
        

```


## Extending CIB Rows

Finally, CIB sources have the option of generating CIB files which augment existing CIB **rows** with additional properties. This can be useful to annotate specific CIB rows with historical information, e.g., collected from previous connections. To achieve this the `link` attribute is set to `false`:

```
{
    "uid": "historic info",
    "description": "appended properties to CIB rows matching all match properties.",
    "priority": 10,
    "timstamp": 1476104788,
    "link": false,
    "match": [
    	{ "interface": {"value": "eth0"}, 
         "local_ip": {"value": "10.10.2.1"}, 
         "remote_ip": {"value": "8.8.8.8"}}        
     ],
    "properties": [{
        "remote_port": {"value": 8080, "precedence":1},
        "local_port": {"value": 56674, "precedence":1},
        "transport": {"value": "TCP", "precedence":1},
        "cached": {"value": true, "precedence":2, "score":5},
        "cache_ttl": {"value": 300, "precedence":1},
        "cache_status": {"value": "connection_success", "precedence":2, "description":"could be failed, NA, etc."}
    }]
}

```

For this example the CIB will match row 1, and insert an **new** row which includes the additional properties.

```
3: {"interface": {"value": "eth0", "precedence":2}, "capacity": {"value": 10000, "precedence":2}, "local_ip": {"value": "10.10.2.1", "precedence":2}, "is_wired": {"value": true, "precedence":2}, "MTU": {"value": {"start":50, "end":9000}, "remote_ip": {"value": "8.8.8.8", "precedence":2, "score": 2}, "remote_port": {"value": 8080, "precedence":1}, "local_port": {"value": 56674, "precedence":1}, "transport": {"value": "TCP", "precedence":1}, "cache_ttl": {"value": 300, "precedence":1}, "cached_connection_status": {"value": "success", "precedence":2, "score":5}}

```