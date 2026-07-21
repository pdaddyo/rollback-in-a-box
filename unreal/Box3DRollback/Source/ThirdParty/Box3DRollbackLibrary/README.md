# Box3DRollbackLibrary (prebuilt static libs)

The Unreal module links two static libraries from here:

- `box3d_rollback_neutral` — the engine-neutral rollback core (repo `/core`)
- `box3d` — the vendored physics engine (repo `/gdext/extern/box3d`)

They are **not committed**. Generate them from the repo root with:

```sh
./unreal/build_thirdparty.sh
```

That produces `lib/<Platform>/` (e.g. `lib/Win64/`, `lib/Mac/`, `lib/Linux/`)
containing the `.lib`/`.a` files `Box3DRollbackLibrary.Build.cs` references.
The runtime module depends on this external module privately so these archives
cannot propagate into another dylib and create a second Box3D global state.
Rebuild whenever the pinned Box3D revision or the core changes.
