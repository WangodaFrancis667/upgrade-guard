# 🚀 Launchpad and PPA Release Procedure

Upgrade Guard `0.1.0-beta.1` maps to Debian version `0.1.0~beta1-1`. Noble and resolute uploads use `0.1.0~beta1-1~ubuntu24.04.1` and `0.1.0~beta1-1~ubuntu26.04.1` respectively.

## 📦 Launchpad Objects

A Launchpad project tracks releases, bugs and translations. A Launchpad Git repository stores source history. A Personal Package Archive (PPA) accepts signed Debian source uploads and builds binary packages. 

They are separate objects and creating one does not create the others. 

> [!NOTE]
> A PPA is not the official Ubuntu archive, and publication there does not imply Ubuntu or Canonical endorsement. Admission to the official archive instead requires Ubuntu packaging review and sponsorship through the Ubuntu development process.

## 🔐 Account and Keys

1. Add an SSH public key to the Launchpad account, verify its fingerprint locally, and test Launchpad Git access. Do not put private keys in this repository.
2. Create or select an OpenPGP signing key with the intended packaging identity. Publish its public key to a supported keyserver, import it into Launchpad, and complete Launchpad's encrypted-message verification.
3. Set `DEBFULLNAME` and `DEBEMAIL` to the identity registered with the key before creating changelog entries. Replace the repository's team-style maintainer only when the project has selected its release identity.
4. Create or select a PPA in Launchpad. Enable amd64. Request or enable arm64 in the PPA settings; arm64 availability is a Launchpad configuration decision and must not be assumed from an amd64 build.

## 🛠️ Build and Verify Source Uploads

Start from a clean, tagged tree and make sure the changelog version is greater than every version previously accepted by the PPA.

```bash
scripts/check-release.sh
scripts/build-source-package.sh noble --key YOUR_KEY_FINGERPRINT
scripts/build-source-package.sh resolute --key YOUR_KEY_FINGERPRINT
debsign --verify artifacts/source/noble/*_source.changes
debsign --verify artifacts/source/resolute/*_source.changes
lintian --pedantic artifacts/source/noble/*_source.changes
lintian --pedantic artifacts/source/resolute/*_source.changes
```

Inspect each `.dsc`, `.orig.tar.gz`, `.debian.tar.*`, `_source.buildinfo` and `_source.changes` file. Launchpad accepts a signed source upload and builds the `.deb`; do not upload a locally built `.deb` directly.

> [!IMPORTANT]
> Upload is intentionally manual. Substitute the real PPA target—never copy a made-up identifier.

```bash
dput ppa:wanpro/upgrade-guard artifacts/source/noble/*_source.changes
dput ppa:wanpro/upgrade-guard artifacts/source/resolute/*_source.changes
```

Watch the PPA's package page for acceptance, dependency resolution, per-architecture builds, publication and test results. Review the Launchpad build log rather than treating upload acceptance as build success.

## 🧪 Install and Lifecycle Test

On disposable noble and resolute VMs, add the selected PPA using the normal Launchpad instructions, refresh package metadata, and test fresh installation, upgrade from the previous PPA version, removal and reinstall. 

Verify `/usr/bin/upgrade-guard`, the man page, Bash completion and JSON schema. Run the installed CLI's version, help, list, scan and JSON-export commands. These package mutations belong only in the disposable validation environment.

> [!WARNING]
> If Launchpad rejects an upload, read the rejection email, fix the source, and increment the Debian revision or Ubuntu suffix before retrying; Launchpad never permits reuse of an already accepted version. After an accepted upload, increment the version for every subsequent upload as well.

Common rejection causes include a reused or lower version, missing or unregistered signature, mismatched distribution series, absent orig tarball, inconsistent source format, unsigned `.changes`, invalid maintainer identity, unsupported compression, build dependencies unavailable in the target series, and a source package containing generated artifacts.
