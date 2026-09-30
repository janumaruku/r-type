#!/bin/bash
set -euo pipefail

ISSUE_KEY="${1:-}"
TARGET_STATUS="${2:-}"

if [[ -z "$ISSUE_KEY" ]]; then
  echo "::error::No issue key provided"
  exit 1
fi

if [[ -z "$TARGET_STATUS" ]]; then
  echo "::error::No target status provided"
  exit 1
fi

# Validate required environment variables
for var in JIRA_EMAIL JIRA_API_TOKEN JIRA_BASE_URL; do
  if [[ -z "${!var:-}" ]]; then
    echo "::error::Missing environment variable: $var"
    exit 1
  fi
done

AUTH_HEADER="$(printf '%s:%s' "$JIRA_EMAIL" "$JIRA_API_TOKEN")"

# Remove trailing slash from base URL if present
JIRA_BASE_URL="${JIRA_BASE_URL%/}"

echo "URL: $JIRA_BASE_URL"

echo "Processing $ISSUE_KEY → $TARGET_STATUS"

# Get available transitions
BODY=$(mktemp)
trap 'rm -f "$BODY"' EXIT

HTTP_CODE=$(curl -s -o "$BODY" -w "%{http_code}" \
  -u "$AUTH_HEADER" \
  -H "Content-Type: application/json" \
  "$JIRA_BASE_URL/rest/api/3/issue/$ISSUE_KEY/transitions")

if [[ "$HTTP_CODE" -ne 200 ]]; then
  echo "::error::Failed to fetch transitions (HTTP $HTTP_CODE): $(cat "$BODY")"
  exit 1
fi

# Extract transition ID (using --arg to safely handle spaces/special chars)
TRANSITION_ID=$(cat "$BODY" | jq -r \
  --arg status "$TARGET_STATUS" \
  '.transitions[] | select(.name == $status) | .id')

if [[ -z "$TRANSITION_ID" ]]; then
  AVAILABLE=$(cat "$BODY" | jq -r '.transitions[].name' | paste -sd', ')
  echo "::error::No transition found for '$TARGET_STATUS'. Available: $AVAILABLE"
  exit 1
fi

# Execute transition
HTTP_CODE=$(curl -s -o "$BODY" -w "%{http_code}" \
  -X POST \
  -u "$AUTH_HEADER" \
  -H "Content-Type: application/json" \
  --data "{\"transition\":{\"id\":\"$TRANSITION_ID\"}}" \
  "$JIRA_BASE_URL/rest/api/3/issue/$ISSUE_KEY/transitions")

if [[ "$HTTP_CODE" -ne 204 ]]; then
  echo "::error::Transition failed (HTTP $HTTP_CODE): $(cat "$BODY")"
  exit 1
fi

echo "::notice::Transitioned $ISSUE_KEY → $TARGET_STATUS"
