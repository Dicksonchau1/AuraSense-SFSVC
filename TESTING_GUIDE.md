# Quick Start Guide - Dashboard Testing

This guide will help you test the AuraSense SFSVC dashboard functionality locally.

## Prerequisites

- Node.js (for mock API server)
- Python 3 (for serving static files)
- A modern web browser

## Setup

### 1. Start the Mock API Server

```bash
node mock-api-server.js
```

You should see:
```
╔════════════════════════════════════════════════════════════╗
║   AuraSense SFSVC Mock API Server                         ║
╚════════════════════════════════════════════════════════════╝

Server running at http://localhost:3000/
```

### 2. Start the Website Server

In a new terminal:

```bash
python3 -m http.server 8080
```

### 3. Open the Test Dashboard

Visit: http://localhost:8080/test-dashboard.html

## Test Scenarios

### Scenario 1: Valid Session (Professional Plan)

**URL:** http://localhost:8080/dashboard.html?session_id=cs_test_1234567890

**Expected Result:**
- Dashboard loads successfully
- Shows "Professional" plan
- Shows 5 drones licensed
- Shows email: demo@aurasensehk.com
- Shows token suffix: a7f9
- Shows 3 mission profiles: FACADE_INSPECTION, BRIDGE_SCAN, BVLOS_CORRIDOR

### Scenario 2: Valid Session (Basic Plan)

**URL:** http://localhost:8080/dashboard.html?session_id=cs_test_basic_plan

**Expected Result:**
- Dashboard loads successfully
- Shows "Basic" plan
- Shows 1 drone licensed
- Shows email: basic@aurasensehk.com

### Scenario 3: Valid Session (Enterprise Plan)

**URL:** http://localhost:8080/dashboard.html?session_id=cs_test_enterprise

**Expected Result:**
- Dashboard loads successfully
- Shows "Enterprise" plan
- Shows 50 drones licensed
- Shows 5 mission profiles

### Scenario 4: Invalid Session Format

**URL:** http://localhost:8080/dashboard.html?session_id=invalid_session

**Expected Result:**
- Error message: "No valid session ID found in URL"
- Suggests checking email for license key

### Scenario 5: Unknown Session

**URL:** http://localhost:8080/dashboard.html?session_id=cs_unknown_session_123

**Expected Result:**
- Error message: "Session not found or expired"
- API returns 404

### Scenario 6: Trial Success Page

**URL:** http://localhost:8080/trial-success.html?session_id=cs_test_1234567890

**Expected Result:**
- Success page loads
- Shows "Professional Plan" label (fetched from API)
- Shows trial confirmation message

## Testing with Different Environments

### Test Production URL Resolution

1. Open browser console
2. Run:
   ```javascript
   window.aurasenseApiUrl('/api/session/test')
   ```
3. Expected result: `http://localhost:3000/api/session/test`

### Test API Override

1. Set override:
   ```javascript
   localStorage.setItem('devApiUrl', 'http://localhost:4000')
   ```
2. Reload dashboard
3. Check network tab - requests should go to localhost:4000

### Clear Override

```javascript
localStorage.removeItem('devApiUrl')
```

## Validation Tests

Use the test dashboard: http://localhost:8080/test-dashboard.html

### Session ID Validation Test

1. Enter various session IDs in the input field
2. Click "Test Validation"
3. Verify:
   - `cs_test_1234567890` → Valid ✓
   - `cs_short` → Invalid ✗ (too short)
   - `invalid_session` → Invalid ✗ (wrong format)
   - `cs_` → Invalid ✗ (too short)

### API URL Configuration Test

1. Click "Test Current Config"
2. Verify API URL resolves to http://localhost:3000
3. Set override to different URL
4. Test again - should use override

### Mock Dashboard Test

1. Click "Render Mock Dashboard"
2. Verify mock data structure matches API response format

## API Endpoints

The mock API server provides:

### GET /health
```bash
curl http://localhost:3000/health
```

Response:
```json
{
  "status": "ok",
  "timestamp": "2026-03-17T10:00:00.000Z"
}
```

### GET /api/session/:sessionId

**Valid Session:**
```bash
curl http://localhost:3000/api/session/cs_test_1234567890
```

Response (200):
```json
{
  "plan": "Professional",
  "drones": 5,
  "email": "demo@aurasensehk.com",
  "token_suffix": "a7f9",
  "mission_profiles": ["FACADE_INSPECTION", "BRIDGE_SCAN", "BVLOS_CORRIDOR"],
  "docker_url": "https://hub.docker.com/r/aurasense/sfsvc",
  "docs_url": "./docs.html",
  "api_ref_url": "./docs.html#api"
}
```

**Invalid Session:**
```bash
curl http://localhost:3000/api/session/invalid_session
```

Response (400):
```json
{
  "error": "Invalid session ID format"
}
```

**Unknown Session:**
```bash
curl http://localhost:3000/api/session/cs_unknown_1234567890
```

Response (404):
```json
{
  "error": "Session not found or expired"
}
```

## Troubleshooting

### Port already in use

If you see "EADDRINUSE" error:

```bash
# Find process using port 3000
lsof -ti:3000

# Kill the process
kill -9 $(lsof -ti:3000)
```

Or change the port in `mock-api-server.js`:
```javascript
const PORT = 3001; // Change this
```

### Dashboard not loading

1. Check browser console for errors
2. Verify both servers are running:
   - API: http://localhost:3000/health
   - Website: http://localhost:8080/
3. Check Network tab in DevTools
4. Verify session_id parameter in URL

### API calls failing

1. Open browser console
2. Check for CORS errors
3. Verify API URL: `window.aurasenseApiUrl('/test')`
4. Check if mock server is running: `curl localhost:3000/health`

## Next Steps

After local testing passes:

1. **Deploy API Server**
   - Implement production `/api/session/:sessionId` endpoint
   - Configure Stripe integration
   - Deploy to https://api.aurasensehk.com

2. **Update Stripe Checkout**
   - Set success_url to include `{CHECKOUT_SESSION_ID}`
   - Test webhook flow

3. **Configure Email Templates**
   - Include dashboard link with session_id
   - Add license key delivery

4. **Deploy to Production**
   - Push changes to main branch
   - GitHub Pages will auto-deploy
   - Test with real Stripe sessions

## Support

For questions or issues:
- Email: support@aurasensehk.com
- Documentation: [DASHBOARD_IMPLEMENTATION.md](./DASHBOARD_IMPLEMENTATION.md)

---

Happy testing! 🚀
