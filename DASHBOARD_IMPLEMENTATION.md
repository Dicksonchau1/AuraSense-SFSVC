# Dashboard & Login Functionality - Implementation Guide

## Overview

This document describes the user login/dashboard functionality for the AuraSense SFSVC licensing system. The system uses **Stripe Checkout sessions** for authentication rather than traditional username/password login.

## Architecture

### Authentication Flow

```
1. User starts free trial → Stripe Checkout
2. Stripe redirects to trial-success.html?session_id=cs_...
3. User receives license key via email
4. User accesses dashboard.html?session_id=cs_... (link in email)
5. Dashboard fetches user data from /api/session/{sessionId}
6. Dashboard renders license details, downloads, and resources
```

### Key Components

#### 1. **trial-success.html**
- Post-checkout success page
- Displays trial confirmation
- Optionally fetches plan details from API
- Provides next steps and documentation links

#### 2. **dashboard.html**
- Main license management interface
- Displays:
  - Active plan
  - Number of licensed drones
  - Account email
  - API token (last 4 characters)
  - Mission profiles
  - Download links (Docker, docs, API reference)

#### 3. **test-dashboard.html**
- Testing and validation tool
- Session ID validation tester
- API URL configuration helper
- Mock data renderer

## Session ID Format

Session IDs must match the pattern: `^cs_[A-Za-z0-9_-]{10,}$`

Examples:
- ✓ Valid: `cs_test_1234567890`
- ✓ Valid: `cs_live_ABcd1234567890_xyz`
- ✗ Invalid: `invalid_session`
- ✗ Invalid: `cs_short`

## API Configuration

The `window.aurasenseApiUrl()` function automatically detects the environment:

### Production
- Domain: `www.aurasensehk.com` or `aurasensehk.com`
- API Base: `https://api.aurasensehk.com`

### Development
- Default: `http://localhost:3000`
- Override: Set `localStorage.devApiUrl = 'http://your-api-url'`

### Usage Example
```javascript
const endpoint = '/api/session/' + sessionId;
const apiUrl = window.aurasenseApiUrl(endpoint);
// Result: https://api.aurasensehk.com/api/session/cs_xxx (production)
// Result: http://localhost:3000/api/session/cs_xxx (development)
```

## API Endpoint

### GET /api/session/:sessionId

**Request:**
```
GET /api/session/cs_test_1234567890
```

**Response (200 OK):**
```json
{
  "plan": "Professional",
  "drones": 5,
  "email": "user@example.com",
  "token_suffix": "a7f9",
  "mission_profiles": [
    "FACADE_INSPECTION",
    "BRIDGE_SCAN",
    "BVLOS_CORRIDOR"
  ],
  "docker_url": "https://hub.docker.com/r/aurasense/sfsvc",
  "docs_url": "https://www.aurasensehk.com/docs.html",
  "api_ref_url": "https://www.aurasensehk.com/docs.html#api"
}
```

**Error Response (400/404):**
```json
{
  "error": "Invalid or expired session"
}
```

## Changes Made

### 1. Fixed Broken Links
- **Before:** Footer links pointed to `./legal/privacy.html` and `./legal/terms.html`
- **After:** Fixed to `./privacy.html` and `./terms.html` (correct paths)

### 2. Made URLs Relative
- **Before:** Navigation used absolute URLs (`https://www.aurasensehk.com/...`)
- **After:** All URLs are now relative (`./index.html`, `./docs.html`, etc.)
- **Benefit:** Works in development and production without modification

### 3. Added API URL Configuration
- **Before:** `window.aurasenseApiUrl` was referenced but never defined
- **After:** Implemented proper environment detection and API URL resolution
- **Features:**
  - Auto-detects production vs development
  - Supports local override via `localStorage.devApiUrl`
  - Provides sensible defaults

### 4. Fixed Download Links
- **Before:** Documentation links used absolute URLs
- **After:** Made relative for consistency

## Testing

### Local Development

1. **Start a local API server** (required for full testing):
   ```bash
   # Example with Node.js/Express
   npm start # Should run on http://localhost:3000
   ```

2. **Serve the website**:
   ```bash
   python3 -m http.server 8080
   # Or use any static file server
   ```

3. **Open test page**:
   ```
   http://localhost:8080/test-dashboard.html
   ```

4. **Test dashboard with mock session**:
   ```
   http://localhost:8080/dashboard.html?session_id=cs_test_1234567890_valid
   ```

### Override API URL for Testing

In browser console:
```javascript
// Point to your local API server
localStorage.setItem('devApiUrl', 'http://localhost:3000');

// Or use a different environment
localStorage.setItem('devApiUrl', 'https://staging-api.aurasensehk.com');

// Clear override
localStorage.removeItem('devApiUrl');
```

## Production Deployment

### Requirements

1. **API Server** must be deployed at `https://api.aurasensehk.com`
2. **Stripe Webhook** must be configured to redirect to:
   - Success: `https://www.aurasensehk.com/trial-success.html?session_id={CHECKOUT_SESSION_ID}`
3. **Email Template** must include dashboard link:
   - Format: `https://www.aurasensehk.com/dashboard.html?session_id={CHECKOUT_SESSION_ID}`

### API Implementation

The API server must implement the `/api/session/:sessionId` endpoint:

```javascript
// Example with Express.js
app.get('/api/session/:sessionId', async (req, res) => {
  const { sessionId } = req.params;

  // Validate session ID format
  if (!/^cs_[A-Za-z0-9_-]{10,}$/.test(sessionId)) {
    return res.status(400).json({ error: 'Invalid session ID format' });
  }

  try {
    // Fetch from Stripe
    const session = await stripe.checkout.sessions.retrieve(sessionId);

    // Fetch subscription details
    const subscription = await stripe.subscriptions.retrieve(
      session.subscription
    );

    // Fetch license details from your database
    const license = await db.getLicenseByCustomer(session.customer);

    // Return formatted response
    res.json({
      plan: subscription.items.data[0].price.nickname,
      drones: license.drone_count,
      email: session.customer_details.email,
      token_suffix: license.token.slice(-4),
      mission_profiles: license.mission_profiles,
      docker_url: license.docker_url,
      docs_url: 'https://www.aurasensehk.com/docs.html',
      api_ref_url: 'https://www.aurasensehk.com/docs.html#api'
    });
  } catch (error) {
    res.status(404).json({ error: 'Session not found or expired' });
  }
});
```

## Security Considerations

1. **Session ID Validation**
   - Client-side validation is a UX improvement, not security
   - Server must validate session IDs with Stripe
   - Expired sessions should return 404

2. **API Token Display**
   - Dashboard only shows last 4 characters
   - Full token delivered via email only
   - Never expose full tokens in API responses

3. **CORS Configuration**
   - API should allow requests from `www.aurasensehk.com`
   - Consider origin validation for production

4. **Rate Limiting**
   - Implement rate limiting on `/api/session/:sessionId`
   - Prevent session ID enumeration attacks

## Troubleshooting

### Dashboard shows "No valid session ID found"
- Check URL contains `?session_id=cs_...`
- Verify session ID matches pattern `^cs_[A-Za-z0-9_-]{10,}$`
- Use test-dashboard.html to validate session ID format

### Dashboard shows "Could not load license details"
- Check browser console for network errors
- Verify API server is running and accessible
- Test API URL configuration with test-dashboard.html
- Check if session ID exists in Stripe

### API URL not resolving correctly
- Open browser console
- Run: `window.aurasenseApiUrl('/api/session/test')`
- Check `localStorage.getItem('devApiUrl')`
- Clear override if needed: `localStorage.removeItem('devApiUrl')`

## Files Modified

1. `dashboard.html` - Main dashboard interface
2. `trial-success.html` - Post-checkout success page
3. `test-dashboard.html` - Testing and validation tool (new)

## Next Steps

1. **Implement API Server**
   - Deploy at `https://api.aurasensehk.com`
   - Implement `/api/session/:sessionId` endpoint
   - Configure CORS and rate limiting

2. **Configure Stripe**
   - Set success URL in Checkout session
   - Test webhook flow

3. **Email Templates**
   - Include dashboard link with session_id
   - Add license key and setup instructions

4. **Monitoring**
   - Set up error logging for API
   - Monitor session validation failures
   - Track dashboard access metrics

## Support

For technical issues or questions:
- Email: support@aurasensehk.com
- Dashboard: https://www.aurasensehk.com/dashboard.html

---

© 2026 AuraSense Limited. All rights reserved.
