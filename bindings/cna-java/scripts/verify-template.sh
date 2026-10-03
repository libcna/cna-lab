#!/usr/bin/env bash
set -euo pipefail

binding_root=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
template_root=${CNA_JAVA_TEMPLATE_ROOT:-"$binding_root/../cna-java-template"}

if [[ ! -x "$template_root/gradlew" ]]; then
    echo "Template Gradle Wrapper not found: $template_root/gradlew" >&2
    exit 2
fi

# Inside this repository's shared, gitignored build-consumer directory rather than /tmp: /tmp is
# the same disk and nothing built there is ever reused. Recreated on every run so the Maven
# repository holds exactly the artifact this run published and nothing older.
verification_root=${CNA_JAVA_VERIFY_ROOT:-"$binding_root/build-consumer/template-verify"}
rm -rf -- "$verification_root"
mkdir -p -- "$verification_root"
repository="$verification_root/maven"
generated="$verification_root/generated"
workers=${CNA_GRADLE_WORKERS:-8}

cleanup() {
    if [[ "${CNA_JAVA_VERIFY_KEEP:-0}" != "1" && -d "$verification_root" ]]; then
        rm -rf -- "$verification_root"
    fi
}
trap cleanup EXIT

echo "[1/4] Building CNA-Java and publishing to $repository"
(
    cd "$binding_root"
    # CNA_JAVA_SKIP_CHECK=1 publishes without re-running `check`, for a tree whose check has
    # just been run separately on the same sources; the default runs it.
    tasks=(clean check publishToMavenLocal)
    if [[ "${CNA_JAVA_SKIP_CHECK:-0}" == "1" ]]; then
        tasks=(clean compileJni publishToMavenLocal)
    fi
    ./gradlew --no-daemon --max-workers="$workers" "${tasks[@]}" \
        "-Dmaven.repo.local=$repository"
)

echo "[2/4] Building sibling template against the exact temporary artifact"
(
    cd "$template_root"
    ./gradlew --no-daemon --max-workers="$workers" clean test installDist "-PcnaRepository=$repository"
)

echo "[3/4] Generating and building a standalone project"
python3 "$template_root/scripts/generate_project.py" \
    --output "$generated" \
    --project-name "Verification Game" \
    --package org.openeggbert.verification \
    --application-id org.openeggbert.verification.desktop \
    --game-class VerificationGame \
    --group org.openeggbert.verification \
    --artifact-id cna-java-generated-verification
(
    cd "$generated"
    ./gradlew --no-daemon --max-workers="$workers" clean test installDist "-PcnaRepository=$repository"
)

echo "[4/4] Native execution: 60-frame smoke, CNA extensions smoke, 600-frame stability run"
if [[ -n "${CNA_NATIVE_LIBRARY:-}" ]]; then
    case "$(uname -s)" in
        Linux*) jni_library="$binding_root/build/native/libcna_java_jni.so" ;;
        Darwin*) jni_library="$binding_root/build/native/libcna_java_jni.dylib" ;;
        MINGW*|MSYS*|CYGWIN*) jni_library="$binding_root/build/native/cna_java_jni.dll" ;;
        *) echo "Unsupported host for JNI test: $(uname -s)" >&2; exit 2 ;;
    esac
    (
        cd "$template_root"
        CNA_JNI_LIBRARY="$jni_library" ./gradlew --no-daemon --max-workers="$workers" :game:run \
            "-PcnaRepository=$repository" --args=--smoke-test
    )
    (
        cd "$template_root"
        CNA_JNI_LIBRARY="$jni_library" ./gradlew --no-daemon --max-workers="$workers" :game:run \
            "-PcnaRepository=$repository" --args=--extensions-smoke
    )
    if [[ "${CNA_SKIP_STABILITY_TEST:-0}" != "1" ]]; then
        (
            cd "$template_root"
            CNA_JNI_LIBRARY="$jni_library" ./gradlew --no-daemon --max-workers="$workers" :game:run \
                "-PcnaRepository=$repository" --args=--stability-test
        )
    else
        echo "Stability run skipped (CNA_SKIP_STABILITY_TEST=1)."
    fi
else
    echo "Native run skipped (CNA_NATIVE_LIBRARY is not set)."
fi

echo "CNA-Java/template verification passed without using the global Maven repository."
