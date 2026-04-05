import type { NavLink } from '../types/content';

export const mainNavLinks: NavLink[] = [
  {
    label: 'About',
    href: '/about',
    children: [
      { label: 'Company', href: '/about' },
      { label: 'Platform', href: '/platform' },
    ],
  },
  {
    label: 'Products',
    href: '/products',
    children: [
      { label: 'SFSVC', href: '/products/sfsvc' },
      { label: 'NERMN', href: '/products/nermn' },
      { label: 'NSSIM', href: '/products/nssim' },
    ],
  },
  {
    label: 'Resources',
    href: '/resources',
  },
  {
    label: 'Careers',
    href: '/careers',
  },
  {
    label: 'Contact',
    href: '/contact',
  },
];
