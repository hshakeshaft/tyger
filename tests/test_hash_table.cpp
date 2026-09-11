///
/// Test Hash Table - this suite is the appropriate place for testing full behaviour
/// of the hash table, utilised for storing identifiers in the VM.
///
/// The exact implementation of said hash table should not be tested in the VM,
/// and as such the VM wraps all hash table functions; however the hash table is
/// integrally linked with the concept of a `TyObject` and thus cannot be divoreced.
///

// TODO: test insertion
//  - [ ] insert lone values
//  - [ ] insert multiple values which don't collide
//  - [ ] insert when hash collision occurs ("separate chaining")

// TODO: test retrieval
//  - [ ] test retrieval when no values in table (===> return NULL)
//  - [ ] test retrieval of lone key which exists
//  - [ ] test retrieval of value when multiple in table (no collisions)
//  - [ ] test retrieval when collision resolution

// TODO: test deletion of key
//  - [ ] deletion of key which doesn't exist (nothing happens - return false)
//  - [ ] test deletion of key which exists (and is lone)
//  - [ ] test deleteion of key which exists when there's several
//    - assert other element not deleted
//  - [ ] test deletion of key which collides
